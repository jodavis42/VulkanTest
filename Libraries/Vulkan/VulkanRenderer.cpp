#include "Precompiled.hpp"

#include "VulkanRenderer.hpp"

#include "Graphics/Mesh.hpp"
#include "Graphics/MaterialShared.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/ZilchShader.hpp"
#include "Graphics/RenderQueue.hpp"
#include "VulkanInitialization.hpp"
#include "VulkanValidationLayers.hpp"
#include "VulkanStructures.hpp"
#include "VulkanImages.hpp"
#include "EnumConversions.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanMaterials.hpp"
#include "VulkanRendering.hpp"

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
  mInternal->mBufferManager.mRuntimeData = mInternal;
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
  for(VulkanMesh* mesh : mMeshMap.Values())
    DestroyMeshInternal(mesh);
  mMeshMap.Clear();

  for(VulkanImage* image : mTextureMap.Values())
    DestroyTextureInternal(image);
  mTextureMap.Clear();

  for(VulkanShader* shader : mZilchShaderMap.Values())
    DestroyShaderInternal(shader);
  mZilchShaderMap.Clear();

  for(VulkanShaderMaterial* shaderMaterial : mUniqueZilchShaderMaterialMap.Values())
    DestroyShaderMaterialInternal(shaderMaterial);
  mUniqueZilchShaderMaterialMap.Clear();

  mInternal->mBufferManager.Destroy();
}

void VulkanRenderer::Shutdown()
{
  for(size_t i = 0; i < mInternal->mMaxFramesInFlight; i++)
  {
    vkDestroyFence(mInternal->mDevice, mInternal->mSyncObjects.mInFlightFences[i], nullptr);
    vkDestroySemaphore(mInternal->mDevice, mInternal->mSyncObjects.mRenderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(mInternal->mDevice, mInternal->mSyncObjects.mImageAvailableSemaphores[i], nullptr);
  }
  if(enableValidationLayers)
    DestroyDebugUtilsMessengerEXT(mInternal->mInstance, mInternal->mDebugMessenger, nullptr);

  vkDestroyCommandPool(mInternal->mDevice, mInternal->mCommandPool, nullptr);

  vkDestroyDevice(mInternal->mDevice, nullptr);
  vkDestroySurfaceKHR(mInternal->mInstance, mInternal->mSurface, nullptr);
  vkDestroyInstance(mInternal->mInstance, nullptr);
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
    VkDeviceSize bufferSize = sizeof(mesh->mVertices[0]) * mesh->mVertices.Size();
    CreateBuffer(vulkanData, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vulkanMesh->mVertexBuffer, vulkanMesh->mVertexBufferMemory, mesh->mVertices.Data(), bufferSize);
  }
  
  {
    vulkanMesh->mIndexCount = static_cast<uint32_t>(mesh->mIndices.Size());
    VkDeviceSize bufferSize = sizeof(mesh->mIndices[0]) * mesh->mIndices.Size();
    CreateBuffer(vulkanData, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vulkanMesh->mIndexBuffer, vulkanMesh->mIndexBufferMemory, mesh->mIndices.Data(), bufferSize);
  }

  mMeshMap[mesh] = vulkanMesh;
}

void VulkanRenderer::DestroyMesh(const Mesh* mesh)
{
  VulkanMesh* vulkanMesh = mMeshMap[mesh];
  mMeshMap.Erase(mesh);

  DestroyMeshInternal(vulkanMesh);
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
  mTextureMap.Erase(texture);
  mTextureNameMap.Erase(texture->mName);

  DestroyTextureInternal(vulkanImage);
}

void VulkanRenderer::CreateShader(const ZilchShader* zilchShader)
{
  VulkanShader* vulkanShader = new VulkanShader();

  vulkanShader->mPixelShaderModule = CreateShaderModule(mInternal->mDevice, zilchShader->mShaderByteCode[ShaderStage::Pixel]);
  vulkanShader->mVertexShaderModule = CreateShaderModule(mInternal->mDevice, zilchShader->mShaderByteCode[ShaderStage::Vertex]);
  vulkanShader->mVertexEntryPointName = zilchShader->mResources[ShaderStage::Vertex].mEntryPointName;
  vulkanShader->mPixelEntryPointName = zilchShader->mResources[ShaderStage::Pixel].mEntryPointName;

  mZilchShaderMap[zilchShader] = vulkanShader;
}

void VulkanRenderer::DestroyShader(const ZilchShader* zilchShader)
{
  VulkanShader* vulkanShader = mZilchShaderMap[zilchShader];
  mZilchShaderMap.Erase(zilchShader);

  DestroyShaderInternal(vulkanShader);
}

void VulkanRenderer::CreateShaderMaterial(ZilchShader* shaderMaterial)
{
  VulkanShaderMaterial* vulkanShaderMaterial = new VulkanShaderMaterial();

  RendererData rendererData{this, mInternal};
  CreateMaterialDescriptorSetLayouts(rendererData, *shaderMaterial, *vulkanShaderMaterial);
  CreateMaterialDescriptorPool(rendererData, *shaderMaterial, *vulkanShaderMaterial);
  CreateMaterialDescriptorSets(rendererData, *vulkanShaderMaterial);

  for(ZilchMaterialBindingDescriptor& bindingDescriptor : shaderMaterial->mBindingDescriptors)
  {
    if(bindingDescriptor.mBufferBindingType == ShaderMaterialBindingId::Material)
      bindingDescriptor.mOffsetInBytes = vulkanShaderMaterial->mBufferOffset;
  }

  mUniqueZilchShaderMaterialMap[shaderMaterial] = vulkanShaderMaterial;
}

void VulkanRenderer::UpdateShaderMaterialInstance(const ZilchShader* zilchShader, const ZilchMaterial* zilchMaterial)
{
  VulkanShaderMaterial* vulkanShaderMaterial = mUniqueZilchShaderMaterialMap[zilchShader];
  VulkanShader* vulkanShader = mZilchShaderMap[zilchShader];

  RendererData rendererData{this, mInternal};
  UpdateMaterialDescriptorSets(rendererData, *zilchShader, *zilchMaterial, *vulkanShaderMaterial);
  CreateGraphicsPipeline(rendererData, *vulkanShader, *vulkanShaderMaterial);
}

void VulkanRenderer::UploadShaderMaterialInstances(MaterialBatchUploadData& materialBatchUploadData)
{
  RendererData rendererData{this, mInternal};
  PopulateMaterialBuffers(rendererData, materialBatchUploadData);
}

void VulkanRenderer::DestroyShaderMaterial(const ZilchShader* zilchShader)
{
  VulkanShaderMaterial* vulkanShaderMaterial = mUniqueZilchShaderMaterialMap[zilchShader];
  mUniqueZilchShaderMaterialMap.Erase(zilchShader);

  DestroyShaderMaterialInternal(vulkanShaderMaterial);
}

RenderFrameStatus VulkanRenderer::BeginFrame()
{
  uint32_t& currentFrame = mInternal->mCurrentFrame;
  auto& syncObjects = mInternal->mSyncObjects;

  vkWaitForFences(mInternal->mDevice, 1, &syncObjects.mInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(mInternal->mDevice, mInternal->mSwapChain.mSwapChain, UINT64_MAX, syncObjects.mImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
  if(result == VK_ERROR_OUT_OF_DATE_KHR)
    return RenderFrameStatus::OutOfDate;
  else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    return RenderFrameStatus::Error;
  
  mInternal->mCurrentImageIndex = imageIndex;
  return RenderFrameStatus::Success;
}

RenderFrameStatus VulkanRenderer::EndFrame()
{
  uint32_t& currentFrame = mInternal->mCurrentFrame;
  uint32_t imageIndex = mInternal->mCurrentImageIndex;
  VulkanRenderFrame& vulkanRenderFrame = mInternal->mRenderFrames[imageIndex];
  auto& syncObjects = mInternal->mSyncObjects;
  if(syncObjects.mImagesInFlight[imageIndex] != VK_NULL_HANDLE)
    vkWaitForFences(mInternal->mDevice, 1, &syncObjects.mImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
  
  syncObjects.mImagesInFlight[imageIndex] = syncObjects.mInFlightFences[currentFrame];

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {syncObjects.mImageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &vulkanRenderFrame.mCommandBuffer;

  VkSemaphore signalSemaphores[] = {syncObjects.mRenderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vkResetFences(mInternal->mDevice, 1, &syncObjects.mInFlightFences[currentFrame]);

  if(vkQueueSubmit(mInternal->mGraphicsQueue, 1, &submitInfo, syncObjects.mInFlightFences[currentFrame]) != VK_SUCCESS)
    return RenderFrameStatus::Error;

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {mInternal->mSwapChain.mSwapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr; // Optional

  VkResult result = vkQueuePresentKHR(mInternal->mPresentQueue, &presentInfo);
  if(result == VK_ERROR_OUT_OF_DATE_KHR)
    return RenderFrameStatus::OutOfDate;
  else if(result == VK_SUBOPTIMAL_KHR)
    return RenderFrameStatus::SubOptimal;
  else if(result != VK_SUCCESS)
    return RenderFrameStatus::Error;

  currentFrame = (currentFrame + 1) % mInternal->mMaxFramesInFlight;
  return RenderFrameStatus::Success;
}

void VulkanRenderer::DrawRenderQueue(RenderQueue& renderQueue)
{
  RendererData rendererData{this, mInternal};
  ProcessRenderQueue(rendererData, renderQueue);
}

void VulkanRenderer::WaitForIdle()
{
  vkDeviceWaitIdle(mInternal->mDevice);
}

void VulkanRenderer::Reshape(size_t width, size_t height, float aspectRatio)
{
  mInternal->mWidth = static_cast<uint32_t>(width);
  mInternal->mHeight = static_cast<uint32_t>(height);
  mInternal->mResized = true;
}

void VulkanRenderer::GetShape(size_t& width, size_t& height, float& aspectRatio) const
{
  width = mInternal->mWidth;
  height = mInternal->mHeight;
  aspectRatio = width / (float)height;
}

Matrix4 VulkanRenderer::BuildPerspectiveMatrix(float verticalFov, float aspectRatio, float nearDistance, float farDistance) const
{
// Near and far distances are expected to be positive
  float depth = farDistance - nearDistance;
  // tan(fov/2) = t/n
  // n/t = cot(fov/2)
  float n_t = 1.0f / std::tan(verticalFov * 0.5f);

  // r = t*(r/t) = t*aspect
  // n/r = n/(t*aspect) = (n/t)/aspect
  float n_r = n_t / aspectRatio;

  Matrix4 m;
  m.SetIdentity();
  m[0][0] = n_r;
  m[1][1] = -n_t;
  m[2][2] = -farDistance / depth;
  m[3][3] = 0.0f;
  m[3][2] = -farDistance * nearDistance / depth;
  m[2][3] = -1.0f;

  return m;
}

void* VulkanRenderer::MapGlobalUniformBufferMemory(const String& bufferName, uint32_t bufferId)
{
  VulkanUniformBuffer* buffer = mInternal->mBufferManager.FindGlobalBuffer(bufferName, bufferId);

  void* data = nullptr;
  if(buffer != nullptr)
    vkMapMemory(mInternal->mDevice, buffer->mBufferMemory, 0, mInternal->mDeviceLimits.mMaxUniformBufferRange, 0, &data);
  return data;
}

void* VulkanRenderer::MapPerFrameUniformBufferMemory(const String& bufferName, uint32_t bufferId, uint32_t frameIndex)
{
  VulkanUniformBuffer* buffer = mInternal->mBufferManager.FindOrCreatePerFrameBuffer(bufferName, bufferId, frameIndex);

  void* data = nullptr;
  if(buffer != nullptr)
    vkMapMemory(mInternal->mDevice, buffer->mBufferMemory, 0, mInternal->mDeviceLimits.mMaxUniformBufferRange, 0, &data);
  return data;
}

void VulkanRenderer::UnMapGlobalUniformBufferMemory(const String& bufferName, uint32_t bufferId)
{
  VulkanUniformBuffer* buffer = mInternal->mBufferManager.FindGlobalBuffer(bufferName, bufferId);

  if(buffer != nullptr && buffer->mBufferMemory != VK_NULL_HANDLE)
    vkUnmapMemory(mInternal->mDevice, buffer->mBufferMemory);
}

void VulkanRenderer::UnMapPerFrameUniformBufferMemory(const String& bufferName, uint32_t bufferId, uint32_t frameIndex)
{
  VulkanUniformBuffer* buffer = mInternal->mBufferManager.FindOrCreatePerFrameBuffer(bufferName, bufferId, frameIndex);

  if(buffer != nullptr && buffer->mBufferMemory != VK_NULL_HANDLE)
    vkUnmapMemory(mInternal->mDevice, buffer->mBufferMemory);
}

size_t VulkanRenderer::AlignUniformBufferOffset(size_t offset)
{
  return ::AlignUniformBufferOffset(mInternal->mDeviceLimits, offset);
}

void VulkanRenderer::DestroyMeshInternal(VulkanMesh* vulkanMesh)
{
  vkDestroyBuffer(mInternal->mDevice, vulkanMesh->mIndexBuffer, nullptr);
  vkFreeMemory(mInternal->mDevice, vulkanMesh->mIndexBufferMemory, nullptr);
  vkDestroyBuffer(mInternal->mDevice, vulkanMesh->mVertexBuffer, nullptr);
  vkFreeMemory(mInternal->mDevice, vulkanMesh->mVertexBufferMemory, nullptr);
}

void VulkanRenderer::DestroyTextureInternal(VulkanImage* vulkanImage)
{
  vkDestroySampler(mInternal->mDevice, vulkanImage->mSampler, nullptr);
  vkDestroyImageView(mInternal->mDevice, vulkanImage->mImageView, nullptr);
  vkFreeMemory(mInternal->mDevice, vulkanImage->mImageMemory, nullptr);
  vkDestroyImage(mInternal->mDevice, vulkanImage->mImage, nullptr);
}

void VulkanRenderer::DestroyShaderInternal(VulkanShader* vulkanShader)
{
  if(vulkanShader == nullptr)
    return;
  vkDestroyShaderModule(mInternal->mDevice, vulkanShader->mPixelShaderModule, nullptr);
  vkDestroyShaderModule(mInternal->mDevice, vulkanShader->mVertexShaderModule, nullptr);
}

void VulkanRenderer::DestroyShaderMaterialInternal(VulkanShaderMaterial* vulkanShaderMaterial)
{
  if(vulkanShaderMaterial == nullptr)
    return;

  vkDestroyPipeline(mInternal->mDevice, vulkanShaderMaterial->mPipeline, nullptr);
  vkDestroyPipelineLayout(mInternal->mDevice, vulkanShaderMaterial->mPipelineLayout, nullptr);
  vkDestroyDescriptorPool(mInternal->mDevice, vulkanShaderMaterial->mDescriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(mInternal->mDevice, vulkanShaderMaterial->mDescriptorSetLayout, nullptr);
  vulkanShaderMaterial->mPipeline = VK_NULL_HANDLE;
  vulkanShaderMaterial->mPipelineLayout = VK_NULL_HANDLE;
  vulkanShaderMaterial->mDescriptorPool = VK_NULL_HANDLE;
  vulkanShaderMaterial->mDescriptorSetLayout = VK_NULL_HANDLE;
  vulkanShaderMaterial->mDescriptorSets.Clear();
  delete vulkanShaderMaterial;
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
  mInternal->mSyncObjects.mImagesInFlight.Resize(mInternal->mSwapChain.GetCount(), VK_NULL_HANDLE);
}

void VulkanRenderer::CreateRenderFramesInternal()
{
  size_t count = mInternal->mSwapChain.mImages.Size();
  mInternal->mRenderFrames.Resize(count);

  Array<VkCommandBuffer> commandBuffers(count);
  CreateCommandBuffer(mInternal->mDevice, mInternal->mCommandPool, commandBuffers.Data(), static_cast<uint32_t>(count));
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
  size_t count = mInternal->mSwapChain.mImages.Size();
  for(VulkanRenderFrame& renderFrame : mInternal->mRenderFrames)
  {
    vkDestroyFramebuffer(mInternal->mDevice, renderFrame.mFrameBuffer, nullptr);
    vkDestroyRenderPass(mInternal->mDevice, renderFrame.mRenderPass, nullptr);
  }
  mInternal->mRenderFrames.Clear();
}

void VulkanRenderer::DestroySwapChainInternal()
{
  if(mInternal->mSwapChain.mImageViews.Empty())
    return;

  for(VkImageView& imageView : mInternal->mSwapChain.mImageViews)
    vkDestroyImageView(mInternal->mDevice, imageView, nullptr);
  mInternal->mSwapChain.mImageViews.Clear();
  mInternal->mSwapChain.mImages.Clear();

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
  textureInfo.mPixels = texture->mTextureData.Data();
  textureInfo.mPixelsSize = static_cast<uint32_t>(texture->mTextureData.Size());
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
