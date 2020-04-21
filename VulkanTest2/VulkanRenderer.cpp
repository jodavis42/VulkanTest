#include "pch.h"

#include "VulkanRenderer.hpp"

#include "VulkanInitialization.hpp"
#include "VulkanValidationLayers.hpp"
#include "VulkanStructures.hpp"
#include "Helpers/Mesh.hpp"
#include "Helpers/Material.hpp"
#include "Helpers/Texture.hpp"
#include "Helpers/Shader.hpp"
#include "Helpers/MaterialBinding.hpp"
#include "VulkanImages.hpp"
#include "Internal/EnumConversions.hpp"
#include "VulkanCommandBuffer.hpp"
#include "Internal/VulkanMaterials.hpp"

VkFramebuffer& FindFrameBuffer(size_t id, VulkanRuntimeData* data)
{
  return data->mRenderFrames[id].mFrameBuffer;
}
VkRenderPass& FindRenderPass(size_t id, VulkanRuntimeData* data)
{
  return data->mRenderFrames[id].mRenderPass;
}
VkCommandBuffer& FindCommandBuffer(size_t id, VulkanRuntimeData* data)
{
  return data->mRenderFrames[id].mCommandBuffer;
}

void CommandBuffer::Begin()
{
  auto data = mRenderFrame->mRenderer->mInternal;
  VkCommandBuffer& vkCommandBuffer = FindCommandBuffer(mId, data);

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0; // Optional
  beginInfo.pInheritanceInfo = nullptr; // Optional

  VulkanStatus result;
  if(vkBeginCommandBuffer(vkCommandBuffer, &beginInfo) != VK_SUCCESS)
    result.MarkFailed("failed to begin recording command buffer!");
}

void CommandBuffer::End()
{
  auto data = mRenderFrame->mRenderer->mInternal;
  VkCommandBuffer& vkCommandBuffer = FindCommandBuffer(mId, data);

  VulkanStatus result;
  if(vkEndCommandBuffer(vkCommandBuffer) != VK_SUCCESS)
    result.MarkFailed("failed to record command buffer!");
}

void CommandBuffer::BeginRenderPass(RenderPass* renderPass)
{
  auto data = mRenderFrame->mRenderer->mInternal;
  VkRenderPass& vkRenderPass = FindRenderPass(renderPass->mId, data);
  VkCommandBuffer& vkCommandBuffer = FindCommandBuffer(mId, data);
  VkFramebuffer& frameBuffer = FindFrameBuffer(renderPass->mTarget->mId, data);

  VkRenderPassBeginInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = vkRenderPass;
  renderPassInfo.framebuffer = frameBuffer;
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = data->mSwapChain.mExtent;
  std::array<VkClearValue, 2> clearValues = {};
  clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(vkCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::EndRenderPass(RenderPass* renderPass)
{
  auto data = mRenderFrame->mRenderer->mInternal;
  VkCommandBuffer& vkCommandBuffer = FindCommandBuffer(mId, data);
  vkCmdEndRenderPass(vkCommandBuffer);
}

RenderFrame::RenderFrame(VulkanRenderer* renderer, size_t id)
{
  mRenderer = renderer;
  mId = id;

  mCommandBuffer.mRenderFrame = this;
  mCommandBuffer.mId = id;

  mRenderTarget.mRenderFrame = this;
  mRenderTarget.mId = id;

  mRenderPass.mRenderFrame = this;
  mRenderPass.mId = id;
  mRenderPass.mTarget = GetFinalRenderTarget();
}

RenderPass* RenderFrame::GetFinalRenderPass()
{
  return &mRenderPass;
}

RenderTarget* RenderFrame::GetFinalRenderTarget()
{
  return &mRenderTarget;
}

RenderTarget* RenderFrame::CreateRenderTarget(Integer2 size)
{
  return nullptr;
}

CommandBuffer* RenderFrame::GetFinalCommandBuffer()
{
  return &mCommandBuffer;
}

CommandBuffer* RenderFrame::CreateCommandBuffer()
{
  return nullptr;
}

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
  CreateRenderFramesInternal();
}

void VulkanRenderer::Cleanup()
{
  DestroyRenderFramesInternal();
  DestroySwapChainInternal();
  DestroyDepthResourcesInternal();
}

void VulkanRenderer::CleanupResources()
{
  while(!mMeshMap.empty())
    DestroyMesh(mMeshMap.begin()->first);
  mMeshMap.clear();

  while(!mTextureMap.empty())
    DestroyTexture(mTextureMap.begin()->first);
  mTextureMap.clear();

  while(!mShaderMap.empty())
    DestroyShader(mShaderMap.begin()->first);
  mShaderMap.clear();

  while(!mShaderMaterialMap.empty())
    DestroyShaderMaterial(mShaderMaterialMap.begin()->first);
  mShaderMaterialMap.clear();

  for(auto pair : mInternal->mMaterialBuffers)
  {
    vkDestroyBuffer(mInternal->mDevice, pair.second.mBuffer, nullptr);
    vkFreeMemory(mInternal->mDevice, pair.second.mBufferMemory, nullptr);
  }
}

void VulkanRenderer::Destroy()
{
  delete mInternal;
}

void VulkanRenderer::CreateMesh(const Mesh* mesh)
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

void VulkanRenderer::DestroyMesh(const Mesh* mesh)
{
  VulkanMesh* vulkanMesh = mMeshMap[mesh];
  mMeshMap.erase(mesh);

  vkDestroyBuffer(mInternal->mDevice, vulkanMesh->mIndexBuffer, nullptr);
  vkFreeMemory(mInternal->mDevice, vulkanMesh->mIndexBufferMemory, nullptr);
  vkDestroyBuffer(mInternal->mDevice, vulkanMesh->mVertexBuffer, nullptr);
  vkFreeMemory(mInternal->mDevice, vulkanMesh->mVertexBufferMemory, nullptr);
}

void VulkanRenderer::CreateTexture(const Texture* texture)
{
  VulkanImage* vulkanImage = new VulkanImage();

  CreateImageInternal(texture, vulkanImage);
  CreateImageViewInternal(texture, vulkanImage);

  mTextureMap[texture] = vulkanImage;
  mTextureNameMap[texture->mName] = vulkanImage;
}

void VulkanRenderer::DestroyTexture(const Texture* texture)
{
  VulkanImage* vulkanImage = mTextureMap[texture];
  mTextureMap.erase(texture);
  mTextureNameMap.erase(texture->mName);

  vkDestroySampler(mInternal->mDevice, vulkanImage->mSampler, nullptr);
  vkDestroyImageView(mInternal->mDevice, vulkanImage->mImageView, nullptr);
  vkFreeMemory(mInternal->mDevice, vulkanImage->mImageMemory, nullptr);
  vkDestroyImage(mInternal->mDevice, vulkanImage->mImage, nullptr);
}

void VulkanRenderer::CreateShader(const Shader* shader)
{
  VulkanShader* vulkanShader = new VulkanShader();

  vulkanShader->mPixelShaderModule = CreateShaderModule(mInternal->mDevice, shader->mShaderByteCode[ShaderStage::Pixel]);
  vulkanShader->mVertexShaderModule = CreateShaderModule(mInternal->mDevice, shader->mShaderByteCode[ShaderStage::Vertex]);

  mShaderMap[shader] = vulkanShader;
}

void VulkanRenderer::DestroyShader(const Shader* shader)
{
  VulkanShader* vulkanShader = mShaderMap[shader];
  mShaderMap.erase(shader);

  vkDestroyShaderModule(mInternal->mDevice, vulkanShader->mPixelShaderModule, nullptr);
  vkDestroyShaderModule(mInternal->mDevice, vulkanShader->mVertexShaderModule, nullptr);
}

void VulkanRenderer::CreateShaderMaterial(const ShaderBinding* shaderBinding)
{
  VulkanShaderMaterial* vulkanShaderMaterial = new VulkanShaderMaterial();

  RendererData rendererData{this, mInternal};
  CreateMaterialDescriptorSetLayouts(rendererData, *shaderBinding, *vulkanShaderMaterial);
  CreateMaterialDescriptorPool(rendererData, *shaderBinding, *vulkanShaderMaterial);
  CreateMaterialDescriptorSets(rendererData, *shaderBinding, *vulkanShaderMaterial);
  
  mShaderMaterialMap[shaderBinding] = vulkanShaderMaterial;
}

void VulkanRenderer::UpdateShaderMaterial(const ShaderMaterialBinding* shaderMaterialBinding)
{
  const Shader* shader = shaderMaterialBinding->mShaderBinding->mShader;
  VulkanShaderMaterial* vulkanShaderMaterial = mShaderMaterialMap[shaderMaterialBinding->mShaderBinding];
  VulkanShader* vulkanShader = mShaderMap[shader];

  RendererData rendererData{this, mInternal};
  UpdateMaterialDescriptorSets(rendererData, *shaderMaterialBinding, *vulkanShaderMaterial);
  CreateGraphicsPipeline(rendererData, *vulkanShader, *vulkanShaderMaterial);
}

void VulkanRenderer::DestroyShaderMaterial(const ShaderBinding* shaderBinding)
{
  VulkanShaderMaterial* vulkanShaderMaterial = mShaderMaterialMap[shaderBinding];
  mShaderMaterialMap.erase(shaderBinding);

  vkDestroyPipeline(mInternal->mDevice, vulkanShaderMaterial->mPipeline, nullptr);
  vkDestroyPipelineLayout(mInternal->mDevice, vulkanShaderMaterial->mPipelineLayout, nullptr);
  vkDestroyDescriptorPool(mInternal->mDevice, vulkanShaderMaterial->mDescriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(mInternal->mDevice, vulkanShaderMaterial->mDescriptorSetLayout, nullptr);
  vulkanShaderMaterial->mPipeline = VK_NULL_HANDLE;
  vulkanShaderMaterial->mPipelineLayout = VK_NULL_HANDLE;
  vulkanShaderMaterial->mDescriptorPool = VK_NULL_HANDLE;
  vulkanShaderMaterial->mDescriptorSetLayout = VK_NULL_HANDLE;
  vulkanShaderMaterial->mDescriptorSets.clear();

  delete vulkanShaderMaterial;
}

RenderFrame* VulkanRenderer::BeginFrame()
{
  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(mInternal->mDevice, mInternal->mSwapChain.mSwapChain, UINT64_MAX, mInternal->mSyncObjects.mImageAvailableSemaphores[mInternal->mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
  if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mInternal->mResized)
  {
    mInternal->mResized = false;
    RecreateFramesInternal();
  }
  else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    throw std::runtime_error("failed to acquire swap chain image!");
  
  RenderFrame* frame = new RenderFrame(this, imageIndex);
  return frame;
}

void VulkanRenderer::EndFrame(RenderFrame* frame)
{
  delete frame;
}

void VulkanRenderer::Resize(size_t width, size_t height)
{
  mInternal->mWidth = static_cast<uint32_t>(width);
  mInternal->mHeight = static_cast<uint32_t>(height);
  mInternal->mResized = true;
}

void VulkanRenderer::Draw()
{

}

void VulkanRenderer::RecreateFramesInternal()
{
  DestroyRenderFramesInternal();
  DestroySwapChainInternal();
  DestroyDepthResourcesInternal();

  CreateDepthResourcesInternal();
  CreateSwapChainInternal();
  CreateRenderFramesInternal();
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
  mInternal->mSyncObjects.mImagesInFlight.resize(mInternal->mSwapChain.GetCount(), VK_NULL_HANDLE);
}

void VulkanRenderer::CreateRenderFramesInternal()
{
  size_t count = mInternal->mSwapChain.mImages.size();
  mInternal->mRenderFrames.resize(count);

  std::vector<VkCommandBuffer> commandBuffers(count);
  CreateCommandBuffer(mInternal->mDevice, mInternal->mCommandPool, commandBuffers.data(), static_cast<uint32_t>(count));
  for(size_t i = 0; i < count; ++i)
  {
    VulkanRenderFrame& vulkanFrame = mInternal->mRenderFrames[i];
    vulkanFrame.mRenderer = this;
    vulkanFrame.mSwapChainImage = mInternal->mSwapChain.mImages[i];
    vulkanFrame.mSwapChainImageView = mInternal->mSwapChain.mImageViews[i];
    vulkanFrame.mCommandBuffer = commandBuffers[i];


    RenderPassCreationData creationData;
    creationData.mRenderPass = vulkanFrame.mRenderPass;
    creationData.mDevice = mInternal->mDevice;
    creationData.mSwapChainImageFormat = mInternal->mSwapChain.mImageFormat;
    creationData.mDepthFormat = mInternal->mDepthFormat;
    CreateRenderPass(creationData);
    vulkanFrame.mRenderPass = creationData.mRenderPass;

    std::array<VkImageView, 2> attachments = {mInternal->mSwapChain.mImageViews[i], mInternal->mDepthImage.mImageView};
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vulkanFrame.mRenderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = mInternal->mSwapChain.mExtent.width;
    framebufferInfo.height = mInternal->mSwapChain.mExtent.height;
    framebufferInfo.layers = 1;

    if(vkCreateFramebuffer(mInternal->mDevice, &framebufferInfo, nullptr, &vulkanFrame.mFrameBuffer) != VK_SUCCESS)
      throw std::runtime_error("failed to create framebuffer!");
  }
}

void VulkanRenderer::DestroyRenderFramesInternal()
{
  size_t count = mInternal->mSwapChain.mImages.size();
  for(VulkanRenderFrame& renderFrame : mInternal->mRenderFrames)
  {
    vkDestroyFramebuffer(mInternal->mDevice, renderFrame.mFrameBuffer, nullptr);
    vkDestroyRenderPass(mInternal->mDevice, renderFrame.mRenderPass, nullptr);
  }
  mInternal->mRenderFrames.clear();
}

void VulkanRenderer::DestroySwapChainInternal()
{
  if(mInternal->mSwapChain.mImageViews.empty())
    return;

  for(auto imageView : mInternal->mSwapChain.mImageViews)
    vkDestroyImageView(mInternal->mDevice, imageView, nullptr);
  mInternal->mSwapChain.mImageViews.clear();
  mInternal->mSwapChain.mImages.clear();

  vkDestroySwapchainKHR(mInternal->mDevice, mInternal->mSwapChain.mSwapChain, nullptr);
}

void VulkanRenderer::CreateImageInternal(const Texture* texture, VulkanImage* image)
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

void VulkanRenderer::CreateImageViewInternal(const Texture* texture, VulkanImage* image)
{
  ImageViewCreationInfo info(mInternal->mDevice, image->mImage);
  info.mFormat = GetImageFormat(texture->mFormat);
  info.mViewType = VK_IMAGE_VIEW_TYPE_2D;
  info.mAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
  info.mMipLevels = static_cast<uint32_t>(texture->mMipLevels);
  VulkanStatus status = CreateImageView(info, image->mImageView);
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
