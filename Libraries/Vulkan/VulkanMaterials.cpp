#include "Precompiled.hpp"

#include "Graphics/MaterialShared.hpp"
#include "Graphics/Vertex.hpp"
#include "Graphics/ZilchShader.hpp"
#include "Graphics/ZilchMaterial.hpp"

#include <vulkan/vulkan.h>
#include "EnumConversions.hpp"
#include "VulkanMaterials.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanStructures.hpp"
#include "VulkanInitialization.hpp"

struct BufferLocation
{
  uint32_t mBufferId;
  size_t mBufferOffset;
};

BufferLocation FindMaterialBufferIdFor(RendererData& rendererData, const ZilchShader& zilchShader)
{
  VulkanUniformBufferManager& bufferManager = rendererData.mRuntimeData->mBufferManager;
  
  uint32_t lastBufferId = bufferManager.GlobalBufferCount(MaterialBufferName) - 1;
  VulkanUniformBuffer* buffer = bufferManager.FindGlobalBuffer(MaterialBufferName, lastBufferId);

  VkDeviceSize requiredSize = 0;
  for(const ZilchMaterialBindingDescriptor& bindingDescriptor : zilchShader.mBindingDescriptors)
  {
    if(bindingDescriptor.mBufferBindingType == ShaderMaterialBindingId::Material)
      requiredSize += bindingDescriptor.mSizeInBytes;
  }

  BufferLocation result;
  if(buffer == nullptr || buffer->mUsedSize + requiredSize >= buffer->mAllocatedSize)
  {
    ++lastBufferId;
    buffer = bufferManager.CreateGlobalBuffer(MaterialBufferName, lastBufferId);
    result.mBufferOffset = buffer->mUsedSize;
  }
  else
    result.mBufferOffset = buffer->mUsedSize;
  buffer->mUsedSize += rendererData.mRenderer->AlignUniformBufferOffset(requiredSize);
  
  result.mBufferId = lastBufferId;
  return result;
}

VkBuffer FindBuffer(RendererData& rendererData, ShaderMaterialBindingId::Enum bufferType, uint32_t frameIndex, uint32_t bufferId)
{
  VulkanUniformBufferManager& bufferManager = rendererData.mRuntimeData->mBufferManager;
  VulkanUniformBuffer* buffer = nullptr;
  if(bufferType == ShaderMaterialBindingId::Global)
    buffer = bufferManager.FindOrCreatePerFrameBuffer(GlobalsBufferName, bufferId, frameIndex);
  else if(bufferType == ShaderMaterialBindingId::Transforms)
    buffer = bufferManager.FindOrCreatePerFrameBuffer(TransformsBufferName, bufferId, frameIndex);
  else if(bufferType == ShaderMaterialBindingId::Material)
    buffer = bufferManager.FindGlobalBuffer(MaterialBufferName, bufferId);
  return buffer->mBuffer;
}

void CreateMaterialDescriptorSetLayouts(RendererData& rendererData, const ZilchShader& zilchShader, VulkanShaderMaterial& vulkanShaderMaterial)
{
  Array<VkDescriptorSetLayoutBinding> layoutBindings;
  layoutBindings.Resize(zilchShader.mBindingDescriptors.Size());

  BufferLocation location = FindMaterialBufferIdFor(rendererData, zilchShader);
  vulkanShaderMaterial.mBufferId = location.mBufferId;
  vulkanShaderMaterial.mBufferOffset = location.mBufferOffset;

  size_t index = 0;
  for(size_t i = 0; i < zilchShader.mBindingDescriptors.Size(); ++i)
  {
    const ZilchMaterialBindingDescriptor& bindingDescriptor = zilchShader.mBindingDescriptors[i];
    VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding = layoutBindings[i];

    descriptorSetLayoutBinding.descriptorCount = 1;
    descriptorSetLayoutBinding.binding = bindingDescriptor.mBindingId;
    descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
    descriptorSetLayoutBinding.descriptorType = ConvertDescriptorType(bindingDescriptor.mDescriptorType);
    // This needs to be controller per shader stage
    descriptorSetLayoutBinding.stageFlags = 0;
    if(bindingDescriptor.mStageFlags & ShaderStageFlags::Vertex)
      descriptorSetLayoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
    if(bindingDescriptor.mStageFlags  & ShaderStageFlags::Pixel)
      descriptorSetLayoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
  }
  
  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.Size());
  layoutInfo.pBindings = layoutBindings.Data();
  
  VulkanStatus result;
  if(vkCreateDescriptorSetLayout(rendererData.mRuntimeData->mDevice, &layoutInfo, nullptr, &vulkanShaderMaterial.mDescriptorSetLayout) != VK_SUCCESS)
    result.MarkFailed("failed to create descriptor set layout!");
}

void CreateMaterialDescriptorPool(RendererData& rendererData, const ZilchShader& zilchShader, VulkanShaderMaterial& vulkanShaderMaterial)
{
  VulkanRuntimeData* runtimeData = rendererData.mRuntimeData;
  uint32_t frameCount = runtimeData->mSwapChain.GetCount();

  std::array<uint32_t, VK_DESCRIPTOR_TYPE_RANGE_SIZE> poolCounts = {};

  size_t index = 0;
  for(const ZilchMaterialBindingDescriptor& bindingDescriptor : zilchShader.mBindingDescriptors)
  {
    if(bindingDescriptor.mDescriptorType == MaterialDescriptorType::Uniform)
      poolCounts[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] += frameCount;
    if(bindingDescriptor.mDescriptorType == MaterialDescriptorType::SampledImage)
      poolCounts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += frameCount;
    if(bindingDescriptor.mDescriptorType == MaterialDescriptorType::UniformDynamic)
      poolCounts[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] += frameCount;
  }

  Array<VkDescriptorPoolSize> poolSizes;
  poolSizes.Reserve(poolCounts.size());
  for(uint32_t i = VK_DESCRIPTOR_TYPE_BEGIN_RANGE; i <= VK_DESCRIPTOR_TYPE_END_RANGE; ++i)
  {
    if(poolCounts[i] != 0)
    {
      VkDescriptorPoolSize poolSize;
      poolSize.type = static_cast<VkDescriptorType>(i);
      poolSize.descriptorCount = poolCounts[i];
      poolSizes.PushBack(poolSize);
    }
  }

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.Size());
  poolInfo.pPoolSizes = poolSizes.Data();
  poolInfo.maxSets = frameCount;

  VulkanStatus status;
  if(vkCreateDescriptorPool(runtimeData->mDevice, &poolInfo, nullptr, &vulkanShaderMaterial.mDescriptorPool) != VK_SUCCESS)
    status.MarkFailed("failed to create descriptor pool!");
}

void CreateMaterialDescriptorSets(RendererData& rendererData, VulkanShaderMaterial& vulkanShaderMaterial)
{
  VulkanRuntimeData* runtimeData = rendererData.mRuntimeData;
  uint32_t frameCount = runtimeData->mSwapChain.GetCount();

  Array<VkDescriptorSetLayout> layouts(frameCount, vulkanShaderMaterial.mDescriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = vulkanShaderMaterial.mDescriptorPool;
  allocInfo.descriptorSetCount = frameCount;
  allocInfo.pSetLayouts = layouts.Data();

  vulkanShaderMaterial.mDescriptorSets.Resize(frameCount);

  VulkanStatus status;
  if(vkAllocateDescriptorSets(runtimeData->mDevice, &allocInfo, vulkanShaderMaterial.mDescriptorSets.Data()) != VK_SUCCESS)
    status.MarkFailed("failed to allocate descriptor sets!");
}

void UpdateMaterialDescriptorSet(RendererData& rendererData, const ZilchShader& zilchShader, const ZilchMaterial& zilchMaterial, VulkanShaderMaterial& vulkanShaderMaterial, size_t frameIndex, VkDescriptorSet descriptorSet)
{
  VulkanRuntimeData* runtimeData = rendererData.mRuntimeData;
  size_t totalCount = zilchShader.mBindingDescriptors.Size();

  Array<VkDescriptorBufferInfo> bufferInfos(totalCount);
  Array<VkDescriptorImageInfo> imageInfos(totalCount);
  Array<VkWriteDescriptorSet> descriptorWrites(totalCount);

  size_t index = 0;
  for(const ZilchMaterialBindingDescriptor& bindingDescriptor: zilchShader.mBindingDescriptors)
  {
    VkBuffer buffer = FindBuffer(rendererData, bindingDescriptor.mBufferBindingType, static_cast<uint32_t>(frameIndex), vulkanShaderMaterial.mBufferId);

    VkWriteDescriptorSet& writeInfo = descriptorWrites[index];
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet = descriptorSet;
    writeInfo.dstBinding = bindingDescriptor.mBindingId;
    writeInfo.dstArrayElement = 0;
    writeInfo.descriptorCount = 1;

    if(bindingDescriptor.mDescriptorType == MaterialDescriptorType::Uniform)
    {
      VkDescriptorBufferInfo& bufferInfo = bufferInfos[index];
      bufferInfo.buffer = buffer;
      bufferInfo.offset = bindingDescriptor.mOffsetInBytes;
      bufferInfo.range = bindingDescriptor.mSizeInBytes;
      writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      writeInfo.pBufferInfo = &bufferInfo;
    }
    else if(bindingDescriptor.mDescriptorType == MaterialDescriptorType::UniformDynamic)
    {
      VkDescriptorBufferInfo& bufferInfo = bufferInfos[index];
      bufferInfo.buffer = buffer;
      bufferInfo.offset = 0;
      bufferInfo.range = bindingDescriptor.mSizeInBytes;
      writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
      writeInfo.pBufferInfo = &bufferInfo;
    }
    else if(bindingDescriptor.mDescriptorType == MaterialDescriptorType::SampledImage)
    {
      VulkanImage* vulkanImage = rendererData.mRenderer->mTextureNameMap[bindingDescriptor.mSampledImageName];
      
      VkDescriptorImageInfo& imageInfo = imageInfos[index];
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = vulkanImage->mImageView;
      imageInfo.sampler = vulkanImage->mSampler;
      writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      writeInfo.pImageInfo = &imageInfo;
    }
    ++index;
  }

  uint32_t descriptorsCount = static_cast<uint32_t>(descriptorWrites.Size());
  vkUpdateDescriptorSets(runtimeData->mDevice, descriptorsCount, descriptorWrites.Data(), 0, nullptr);
}

void UpdateMaterialDescriptorSets(RendererData& rendererData, const ZilchShader& zilchShader, const ZilchMaterial& zilchMaterial, VulkanShaderMaterial& vulkanShaderMaterial)
{
  VulkanRuntimeData* runtimeData = rendererData.mRuntimeData;
  for(size_t i = 0; i < runtimeData->mSwapChain.GetCount(); ++i)
    UpdateMaterialDescriptorSet(rendererData, zilchShader, zilchMaterial, vulkanShaderMaterial, i, vulkanShaderMaterial.mDescriptorSets[i]);
}

void CreateGraphicsPipeline(RendererData& rendererData, const VulkanShader& vulkanShader, VulkanShaderMaterial& vulkanShaderMaterial)
{
  VulkanRuntimeData* runtimeData = rendererData.mRuntimeData;
  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  CreatePipelineLayout(runtimeData->mDevice, &vulkanShaderMaterial.mDescriptorSetLayout, 1, vulkanShaderMaterial.mPipelineLayout);

  GraphicsPipelineCreationInfo creationInfo;
  creationInfo.mVertexShaderModule = vulkanShader.mVertexShaderModule;
  creationInfo.mPixelShaderModule = vulkanShader.mPixelShaderModule;
  creationInfo.mVertexShaderMainFnName = vulkanShader.mVertexEntryPointName;
  creationInfo.mPixelShaderMainFnName = vulkanShader.mPixelEntryPointName;
  creationInfo.mDevice = runtimeData->mDevice;
  creationInfo.mPipelineLayout = vulkanShaderMaterial.mPipelineLayout;
  creationInfo.mRenderPass = runtimeData->mRenderFrames[0].mRenderPass;
  creationInfo.mViewportSize = Vec2((float)runtimeData->mSwapChain.mExtent.width, (float)runtimeData->mSwapChain.mExtent.height);
  creationInfo.mVertexAttributeDescriptions = VulkanVertex::getAttributeDescriptions();
  creationInfo.mVertexBindingDescriptions = VulkanVertex::getBindingDescription();
  CreateGraphicsPipeline(creationInfo, vulkanShaderMaterial.mPipeline);
}

//void DestroyVulkanPipeline(RendererData& rendererData, VulkanMaterialPipeline* vulkanPipeline)
//{
//  vkDestroyPipeline(rendererData.mRuntimeData->mDevice, vulkanPipeline->mPipeline, nullptr);
//  vkDestroyPipelineLayout(rendererData.mRuntimeData->mDevice, vulkanPipeline->mPipelineLayout, nullptr);
//  vkDestroyDescriptorPool(rendererData.mRuntimeData->mDevice, vulkanPipeline->mDescriptorPool, nullptr);
//}
