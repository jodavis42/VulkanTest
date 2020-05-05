#include "Precompiled.hpp"

#include "Graphics/Material.hpp"
#include "Graphics/MaterialBinding.hpp"
#include "Graphics/Vertex.hpp"

#include <vulkan/vulkan.h>
#include "EnumConversions.hpp"
#include "VulkanMaterials.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanStructures.hpp"
#include "VulkanInitialization.hpp"

void AllocateUniformBuffer(VulkanRuntimeData* runtimeData, VulkanUniformBuffer& buffer)
{
  buffer.mAllocatedSize = runtimeData->mDeviceLimits.mMaxUniformBufferRange;
  buffer.mUsedSize = 0;
  VulkanBufferCreationData vulkanData{runtimeData->mPhysicalDevice, runtimeData->mDevice, runtimeData->mGraphicsQueue, runtimeData->mCommandPool};
  VkBufferUsageFlags usageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  CreateBuffer(vulkanData, buffer.mAllocatedSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, usageFlags, buffer.mBuffer, buffer.mBufferMemory);
}

uint32_t FindMaterialBufferIdFor(RendererData& rendererData, const UniqueShaderMaterial& uniqueShaderMaterial)
{
  uint32_t& lastBufferId = rendererData.mRuntimeData->mLastUsedMaterialBufferId;
  auto& materialBuffers = rendererData.mRuntimeData->mMaterialBuffers;
  VulkanUniformBuffer* buffer = nullptr;

  if(materialBuffers.ContainsKey(lastBufferId))
  {
    buffer = &materialBuffers[lastBufferId];
  }

  VkDeviceSize requiredSize = 0;
  for(ShaderResourceBinding * shaderResource : uniqueShaderMaterial.mBindings.Values())
  {
    if(shaderResource->mMaterialBindingId == ShaderMaterialBindingId::Material)
      requiredSize += shaderResource->mBoundResource->mSizeInBytes;
  }
  if(buffer == nullptr || buffer->mUsedSize + requiredSize >= buffer->mAllocatedSize)
  {
    ++lastBufferId;
    buffer = &rendererData.mRuntimeData->mMaterialBuffers[lastBufferId];
    AllocateUniformBuffer(rendererData.mRuntimeData, *buffer);
  }
  
  return lastBufferId;
}

VulkanUniformBuffer& FindMaterialBuffer(RendererData& rendererData, uint32_t bufferId)
{
  VulkanRuntimeData* runtimeData = rendererData.mRuntimeData;

  auto& buffers = runtimeData->mMaterialBuffers;
  VulkanUniformBuffer& buffer = buffers[bufferId];
  return buffer;
}

void CreateMaterialDescriptorSetLayouts(RendererData& rendererData, const UniqueShaderMaterial& uniqueShaderMaterial, VulkanShaderMaterial& vulkanShaderMaterial)
{
  Array<VkDescriptorSetLayoutBinding> layoutBindings;
  layoutBindings.Resize(uniqueShaderMaterial.mBindings.Size());
  
  size_t index = 0;
  for(const ShaderResourceBinding* shaderResourceBinding : uniqueShaderMaterial.mBindings.Values())
  {
    const ShaderResource* shaderResource = shaderResourceBinding->mBoundResource;
    VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding = layoutBindings[index];
    ++index;

    descriptorSetLayoutBinding.descriptorCount = 1;
    descriptorSetLayoutBinding.binding = static_cast<uint32_t>(shaderResource->mBindingId);
    descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
    descriptorSetLayoutBinding.descriptorType = ConvertDescriptorType(shaderResourceBinding->mDescriptorType);
    descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  }
  
  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.Size());
  layoutInfo.pBindings = layoutBindings.Data();
  
  VulkanStatus result;
  if(vkCreateDescriptorSetLayout(rendererData.mRuntimeData->mDevice, &layoutInfo, nullptr, &vulkanShaderMaterial.mDescriptorSetLayout) != VK_SUCCESS)
    result.MarkFailed("failed to create descriptor set layout!");

  vulkanShaderMaterial.mBufferId = FindMaterialBufferIdFor(rendererData, uniqueShaderMaterial);
}

void CreateMaterialDescriptorPool(RendererData& rendererData, const UniqueShaderMaterial& uniqueShaderMaterial, VulkanShaderMaterial& vulkanShaderMaterial)
{
  VulkanRuntimeData* runtimeData = rendererData.mRuntimeData;
  uint32_t frameCount = runtimeData->mSwapChain.GetCount();

  std::array<uint32_t, VK_DESCRIPTOR_TYPE_RANGE_SIZE> poolCounts = {};

  size_t index = 0;
  for(ShaderResourceBinding * resourceBinding : uniqueShaderMaterial.mBindings.Values())
  {
    if(resourceBinding->mDescriptorType == MaterialDescriptorType::Uniform)
      poolCounts[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] += frameCount;
    if(resourceBinding->mDescriptorType == MaterialDescriptorType::SampledImage)
      poolCounts[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += frameCount;
    if(resourceBinding->mDescriptorType == MaterialDescriptorType::UniformDynamic)
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

void UpdateMaterialDescriptorSet(RendererData& rendererData, const ShaderMaterialInstance& shaderMaterialInstance, VulkanShaderMaterial& vulkanShaderMaterial, size_t frameIndex, VkDescriptorSet descriptorSet)
{
  const UniqueShaderMaterial* uniqueShaderMaterial = shaderMaterialInstance.mUniqueShaderMaterial;
  VulkanRuntimeData* runtimeData = rendererData.mRuntimeData;
  size_t totalCount = uniqueShaderMaterial->mBindings.Size();

  Array<VkDescriptorBufferInfo> bufferInfos(totalCount);
  Array<VkDescriptorImageInfo> imageInfos(totalCount);
  Array<VkWriteDescriptorSet> descriptorWrites(totalCount);

  size_t index = 0;
  for(ShaderResourceBinding* resourceBinding : uniqueShaderMaterial->mBindings.Values())
  {
    VulkanUniformBuffers* buffers = rendererData.mRenderer->RequestUniformBuffer(0);
    VkBuffer buffer = buffers->mBuffers[frameIndex].mBuffer;
    if(resourceBinding->mMaterialBindingId == ShaderMaterialBindingId::Material)
      buffer = FindMaterialBuffer(rendererData, vulkanShaderMaterial.mBufferId).mBuffer;

    VkWriteDescriptorSet& writeInfo = descriptorWrites[index];
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.dstSet = descriptorSet;
    writeInfo.dstBinding = static_cast<uint32_t>(resourceBinding->mBoundResource->mBindingId);
    writeInfo.dstArrayElement = 0;
    writeInfo.descriptorCount = 1;

    if(resourceBinding->mDescriptorType == MaterialDescriptorType::Uniform)
    {
      VkDescriptorBufferInfo& bufferInfo = bufferInfos[index];
      bufferInfo.buffer = buffer;
      bufferInfo.offset = resourceBinding->mBufferOffset;
      bufferInfo.range = resourceBinding->mBoundResource->mSizeInBytes;
      writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      writeInfo.pBufferInfo = &bufferInfo;
    }
    else if(resourceBinding->mDescriptorType == MaterialDescriptorType::UniformDynamic)
    {
      VkDescriptorBufferInfo& bufferInfo = bufferInfos[index];
      bufferInfo.buffer = buffer;
      bufferInfo.offset = resourceBinding->mBufferOffset;
      bufferInfo.range = resourceBinding->mBoundResource->mSizeInBytes;
      writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
      writeInfo.pBufferInfo = &bufferInfo;
    }
    else if(resourceBinding->mDescriptorType == MaterialDescriptorType::SampledImage)
    {
      ShaderFieldBinding* fieldBinding = shaderMaterialInstance.mMaterialNameMap.FindValue(resourceBinding->mBindingName, nullptr);
      const MaterialProperty* materialProp = fieldBinding->mMaterialProperty;
      String textureName((const char*)materialProp->mData.Data());
      VulkanImage* vulkanImage = rendererData.mRenderer->mTextureNameMap[textureName];

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

void UpdateMaterialDescriptorSets(RendererData& rendererData, const ShaderMaterialInstance& shaderMaterialInstance, VulkanShaderMaterial& vulkanShaderMaterial)
{
  VulkanRuntimeData* runtimeData = rendererData.mRuntimeData;
  for(size_t i = 0; i < runtimeData->mSwapChain.GetCount(); ++i)
    UpdateMaterialDescriptorSet(rendererData, shaderMaterialInstance, vulkanShaderMaterial, i, vulkanShaderMaterial.mDescriptorSets[i]);
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
