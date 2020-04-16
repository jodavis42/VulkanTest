#include "pch.h"
#pragma optimize("", off)
#include "VulkanRenderer.hpp"

#include "VulkanInitialization.hpp"
#include "VulkanValidationLayers.hpp"
#include "VulkanStructures.hpp"
#include "Helpers/Mesh.hpp"
#include "Helpers/Material.hpp"
#include "Helpers/Texture.hpp"
#include "Helpers/Shader.hpp"
#include "VulkanImages.hpp"
#include "Internal/EnumConversions.hpp"

VulkanRenderer::VulkanRenderer()
{
  mInternal = new VulkanRuntimeData();
}

VulkanRenderer::~VulkanRenderer()
{
  Destroy();
}

void VulkanRenderer::Initialize(const VulkanInitializationData& initData)
{
  mInternal->mWidth = static_cast<uint32_t>(initData.mWidth);
  mInternal->mHeight = static_cast<uint32_t>(initData.mHeight);
  mInternal->mSurfaceCreationCallback = initData.mSurfaceCreationCallback;
  InitializeVulkan(*mInternal);
  CreateDepthResourcesInternal();
  CreateSwapChainInternal();
}

void VulkanRenderer::Cleanup()
{
  DestroyDepthResourcesInternal();
}

void VulkanRenderer::Destroy()
{
  
  delete mInternal;
}

void VulkanRenderer::CreateMesh(Mesh* mesh)
{
  VulkanMesh* vulkanMesh = new VulkanMesh();

  VulkanBufferCreationData vulkanData;
  vulkanData.mPhysicalDevice = mInternal->mPhysicalDevice;
  vulkanData.mDevice = mInternal->mDevice;
  vulkanData.mCommandPool = mInternal->mCommandPool;
  vulkanData.mGraphicsQueue = mInternal->mGraphicsQueue;

  {
    VkDeviceSize bufferSize = sizeof(mesh->mVertices[0]) * mesh->mVertices.size();
    CreateBuffer(vulkanData, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vulkanMesh->mVertexBuffer, vulkanMesh->mVertexBufferMemory, mesh->mVertices.data(), bufferSize);
  }
  
  {
    vulkanMesh->mIndexCount = static_cast<uint32_t>(mesh->mIndices.size());
    VkDeviceSize bufferSize = sizeof(mesh->mIndices[0]) * mesh->mIndices.size();
    CreateBuffer(vulkanData, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vulkanMesh->mIndexBuffer, vulkanMesh->mIndexBufferMemory, mesh->mIndices.data(), bufferSize);
  }

  mMeshMap[mesh] = vulkanMesh;
}

void VulkanRenderer::DestroyMesh(Mesh* mesh)
{
  VulkanMesh* vulkanMesh = mMeshMap[mesh];
  mMeshMap.erase(mesh);

  vkDestroyBuffer(mInternal->mDevice, vulkanMesh->mIndexBuffer, nullptr);
  vkFreeMemory(mInternal->mDevice, vulkanMesh->mIndexBufferMemory, nullptr);
  vkDestroyBuffer(mInternal->mDevice, vulkanMesh->mVertexBuffer, nullptr);
  vkFreeMemory(mInternal->mDevice, vulkanMesh->mVertexBufferMemory, nullptr);
}

void VulkanRenderer::CreateTexture(Texture* texture)
{
  VulkanImage* vulkanImage = new VulkanImage();

  CreateImageInternal(texture, vulkanImage);
  CreateImageViewInternal(texture, vulkanImage);

  mTextureMap[texture] = vulkanImage;
}

void VulkanRenderer::DestroyTexture(Texture* texture)
{
  VulkanImage* vulkanImage = mTextureMap[texture];
  mTextureMap.erase(texture);

  vkDestroySampler(mInternal->mDevice, vulkanImage->mSampler, nullptr);
  vkDestroyImageView(mInternal->mDevice, vulkanImage->mImageView, nullptr);
  vkFreeMemory(mInternal->mDevice, vulkanImage->mImageMemory, nullptr);
  vkDestroyImage(mInternal->mDevice, vulkanImage->mImage, nullptr);
}

void VulkanRenderer::CreateShader(Shader* shader)
{
  VulkanShader* vulkanShader = new VulkanShader();

  vulkanShader->mPixelShaderModule = CreateShaderModule(mInternal->mDevice, shader->mShaderByteCode[ShaderStage::Pixel]);
  vulkanShader->mVertexShaderModule = CreateShaderModule(mInternal->mDevice, shader->mShaderByteCode[ShaderStage::Vertex]);

  mShaderMap[shader] = vulkanShader;
}

void VulkanRenderer::DestroyShader(Shader* shader)
{
  VulkanShader* vulkanShader = mShaderMap[shader];
  mShaderMap.erase(shader);

  vkDestroyShaderModule(mInternal->mDevice, vulkanShader->mPixelShaderModule, nullptr);
  vkDestroyShaderModule(mInternal->mDevice, vulkanShader->mVertexShaderModule, nullptr);
}

void VulkanRenderer::CreateMaterial(Material* material, MaterialDescriptorSetLayout* globalDescriptors, size_t globalCount)
{
  VulkanMaterial* vulkanMaterial = new VulkanMaterial();
  
  std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
  layoutBindings.resize(globalCount + material->mDescriptorLayouts.size());
  for(size_t i = 0; i < globalCount; ++i)
  {
    MaterialDescriptorSetLayout& globalDescriptorSet = globalDescriptors[i];
    VkDescriptorSetLayoutBinding& vkBinding = layoutBindings[i];
    SetDescriptorSetLayoutBinding(&globalDescriptorSet, &vkBinding);
  }
  
  for(size_t i = 0; i < material->mDescriptorLayouts.size(); ++i)
  {
    MaterialDescriptorSetLayout& materialDescriptorSet = material->mDescriptorLayouts[i];
    VkDescriptorSetLayoutBinding& vkBinding = layoutBindings[i + globalCount];
    SetDescriptorSetLayoutBinding(&materialDescriptorSet, &vkBinding);
    if(materialDescriptorSet.mDescriptorType == MaterialDescriptorType::Uniform ||
      materialDescriptorSet.mDescriptorType == MaterialDescriptorType::UniformDynamic)
    {
      // Fix this!!
      vulkanMaterial->mBufferSize = static_cast<uint32_t>(materialDescriptorSet.mTotalSize);
    }
  }
  
  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
  layoutInfo.pBindings = layoutBindings.data();
  
  VulkanStatus result;
  if(vkCreateDescriptorSetLayout(mInternal->mDevice, &layoutInfo, nullptr, &vulkanMaterial->mDescriptorSetLayout) != VK_SUCCESS)
    result.MarkFailed("failed to create descriptor set layout!");

  VulkanShader* vulkanShader = mShaderMap[material->mShader];
  vulkanMaterial->mVertexShaderModule = vulkanShader->mVertexShaderModule;
  vulkanMaterial->mPixelShaderModule = vulkanShader->mPixelShaderModule;

  mMaterialMap[material] = vulkanMaterial;
}

void VulkanRenderer::DestroyMaterial(Material* material)
{
  VulkanMaterial* vulkanMaterial = mMaterialMap[material];
  mMaterialMap.erase(material);

  vkDestroyDescriptorSetLayout(mInternal->mDevice, vulkanMaterial->mDescriptorSetLayout, nullptr);
}

void VulkanRenderer::Resize(size_t width, size_t height)
{
  DestroyDepthResourcesInternal();
  mInternal->mWidth = static_cast<uint32_t>(width);
  mInternal->mHeight = static_cast<uint32_t>(height);
  CreateDepthResourcesInternal();
}

void VulkanRenderer::Draw()
{

}

void VulkanRenderer::CreateSwapChainInternal()
{
  SwapChainCreationInfo swapChainInfo;
  swapChainInfo.mDevice = mInternal->mDevice;
  swapChainInfo.mPhysicalDevice = mInternal->mPhysicalDevice;
  swapChainInfo.mSurface = mInternal->mSurface;
  swapChainInfo.mExtent = Integer2(mInternal->mWidth, mInternal->mHeight);

  SwapChainResultInfo swapChainResultInfo;
  CreateSwapChainAndViews(swapChainInfo, mInternal->mSwapChain);
}

void VulkanRenderer::DestroySwapChainInternal()
{
  for(auto imageView : mInternal->mSwapChain.mImageViews)
    vkDestroyImageView(mInternal->mDevice, imageView, nullptr);

  vkDestroySwapchainKHR(mInternal->mDevice, mInternal->mSwapChain.mSwapChain, nullptr);
}

void VulkanRenderer::CreateImageInternal(Texture* texture, VulkanImage* image)
{
  TextureImageCreationInfo textureInfo;
  textureInfo.mPhysicalDevice = mInternal->mPhysicalDevice;
  textureInfo.mDevice = mInternal->mDevice;
  textureInfo.mGraphicsQueue = mInternal->mGraphicsQueue;
  textureInfo.mGraphicsPipeline = mInternal->mGraphicsPipeline;
  textureInfo.mCommandPool = mInternal->mCommandPool;
  textureInfo.mFormat = GetImageFormat(texture->mFormat);
  textureInfo.mPixels = texture->mTextureData.data();
  textureInfo.mPixelsSize = static_cast<uint32_t>(texture->mTextureData.size());
  textureInfo.mWidth = static_cast<uint32_t>(texture->mSizeX);
  textureInfo.mHeight = static_cast<uint32_t>(texture->mSizeY);
  textureInfo.mMipLevels = static_cast<uint32_t>(texture->mMipLevels);
  CreateTextureImage(textureInfo, *image);

  SamplerCreationInfo samplerInfo;
  samplerInfo.mDevice = mInternal->mDevice;
  samplerInfo.mMaxLod = static_cast<float>(texture->mMipLevels);
  samplerInfo.mAddressingU = ConvertSamplerAddressMode(texture->mAddressingX);
  samplerInfo.mAddressingV = ConvertSamplerAddressMode(texture->mAddressingY);
  samplerInfo.mMinFilter = ConvertFilterMode(texture->mMinFilter);
  samplerInfo.mMagFilter = ConvertFilterMode(texture->mMagFilter);
  CreateTextureSampler(samplerInfo, image->mSampler);
}

void VulkanRenderer::CreateImageViewInternal(Texture* texture, VulkanImage* image)
{
  ImageViewCreationInfo info(mInternal->mDevice, image->mImage);
  info.mFormat = GetImageFormat(texture->mFormat);
  info.mViewType = VK_IMAGE_VIEW_TYPE_2D;
  info.mAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
  info.mMipLevels = static_cast<uint32_t>(texture->mMipLevels);
  VulkanStatus status = CreateImageView(info, image->mImageView);
}

void VulkanRenderer::SetDescriptorSetLayoutBinding(MaterialDescriptorSetLayout* materialDescriptor, void* vulkanDescriptor)
{
  VkDescriptorSetLayoutBinding* vkBinding = (VkDescriptorSetLayoutBinding*)vulkanDescriptor;
  vkBinding->descriptorCount = 1;
  vkBinding->binding = static_cast<uint32_t>(materialDescriptor->mBinding);
  vkBinding->pImmutableSamplers = nullptr;
  vkBinding->descriptorType = ConvertDescriptorType(materialDescriptor->mDescriptorType);
  vkBinding->stageFlags = ConvertStageFlags(materialDescriptor->mStageFlags);
}

void VulkanRenderer::CreateDepthResourcesInternal()
{
  mInternal->mDepthFormat = FindDepthFormat(mInternal->mPhysicalDevice);

  ImageCreationInfo imageInfo;
  imageInfo.mDevice = mInternal->mDevice;
  imageInfo.mWidth = mInternal->mWidth;
  imageInfo.mHeight = mInternal->mHeight;
  imageInfo.mMipLevels = 1;
  imageInfo.mFormat = mInternal->mDepthFormat;
  imageInfo.mTiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.mUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  imageInfo.mType = VK_IMAGE_TYPE_2D;
  CreateImage(imageInfo, mInternal->mDepthImage.mImage);

  ImageMemoryCreationInfo memoryInfo;
  memoryInfo.mImage = mInternal->mDepthImage.mImage;
  memoryInfo.mDevice = mInternal->mDevice;
  memoryInfo.mPhysicalDevice = mInternal->mPhysicalDevice;
  memoryInfo.mProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  CreateImageMemory(memoryInfo, mInternal->mDepthImage.mImageMemory);

  vkBindImageMemory(mInternal->mDevice, mInternal->mDepthImage.mImage, mInternal->mDepthImage.mImageMemory, 0);

  ImageViewCreationInfo viewCreationInfo(mInternal->mDevice, mInternal->mDepthImage.mImage);
  viewCreationInfo.mFormat = mInternal->mDepthFormat;
  viewCreationInfo.mViewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCreationInfo.mAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
  viewCreationInfo.mMipLevels = 1;
  VulkanStatus status = CreateImageView(viewCreationInfo, mInternal->mDepthImage.mImageView);

  ImageLayoutTransitionInfo transitionInfo;
  transitionInfo.mDevice = mInternal->mDevice;
  transitionInfo.mGraphicsQueue = mInternal->mGraphicsQueue;
  transitionInfo.mCommandPool = mInternal->mCommandPool;
  transitionInfo.mFormat = mInternal->mDepthFormat;
  transitionInfo.mImage = mInternal->mDepthImage.mImage;
  transitionInfo.mOldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  transitionInfo.mNewLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  transitionInfo.mMipLevels = 1;
  TransitionImageLayout(transitionInfo);
}

void VulkanRenderer::DestroyDepthResourcesInternal()
{
  ::Cleanup(mInternal->mDevice, mInternal->mDepthImage);
}
