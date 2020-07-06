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
#include "VulkanBuffer.hpp"
#include "VulkanSampler.hpp"
#include "VulkanImage.hpp"
#include "VulkanImages.hpp"
#include "VulkanImageView.hpp"
#include "EnumConversions.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanMaterials.hpp"
#include "VulkanRendering.hpp"

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
  CreateDefaultRenderPass();
  CreateRenderFramesInternal();
}

void VulkanRenderer::Cleanup()
{
  DestroyRenderFramesInternal();
  DestroyDefaultRenderPass();
  DestroySwapChainInternal();
  DestroyDepthResourcesInternal();
}

void VulkanRenderer::CleanupResources()
{
  for(VulkanMesh* mesh : mMeshMap.Values())
    DestroyMeshInternal(mesh);
  mMeshMap.Clear();

  for(VulkanTexturedImageData* texturedImageData : mTextureMap.Values())
    DestroyTextureInternal(texturedImageData);
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
  mInternal->mResourcePool.Free(*mInternal);
  mInternal->mAllocator->FreeAllAllocations();
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
  delete mInternal->mAllocator;
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
  VulkanTexturedImageData* texturedImageData = new VulkanTexturedImageData();
  texturedImageData->mSampler = CreateSamplerInternal(texture);
  texturedImageData->mImage = CreateImageInternal(texture);
  CreateImageMemoryInternal(texture, texturedImageData->mImage, *texturedImageData);
  texturedImageData->mImageView = CreateImageViewInternal(texture, texturedImageData->mImage);

  mTextureMap[texture] = texturedImageData;
  mTextureNameMap[texture->mName] = texturedImageData;
}

void VulkanRenderer::DestroyTexture(const Texture* texture)
{
  VulkanTexturedImageData* texturedImageData = mTextureMap[texture];
  mTextureMap.Erase(texture);
  mTextureNameMap.Erase(texture->mName);

  DestroyTextureInternal(texturedImageData);
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
  VkResult result = vkAcquireNextImageKHR(mInternal->mDevice, mInternal->mSwapChain->GetVulkanSwapChain(), UINT64_MAX, syncObjects.mImageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
  if(result == VK_ERROR_OUT_OF_DATE_KHR)
    return RenderFrameStatus::OutOfDate;
  else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    return RenderFrameStatus::Error;
  
  mInternal->mCurrentImageIndex = imageIndex;
  
  // Free any one-time resources for the frame now that we know it's finished
  VulkanRenderFrame& renderFrame = mInternal->mRenderFrames[imageIndex];
  renderFrame.mResources.Free(*mInternal);
  renderFrame.mResources.Clear();

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
  VkCommandBuffer commandBuffer = vulkanRenderFrame.mCommandBuffer->GetVulkanCommandBuffer();
  submitInfo.pCommandBuffers = &commandBuffer;

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

  VkSwapchainKHR swapChains[] = {mInternal->mSwapChain->GetVulkanSwapChain()};
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

void VulkanRenderer::BeginReshape()
{
  DestroyRenderFramesInternal();
  DestroyDefaultRenderPass();
  DestroySwapChainInternal();
  DestroyDepthResourcesInternal();
}

void VulkanRenderer::Reshape(size_t width, size_t height, float aspectRatio)
{
  mInternal->mWidth = static_cast<uint32_t>(width);
  mInternal->mHeight = static_cast<uint32_t>(height);
  mInternal->mResized = true;
}

void VulkanRenderer::EndReshape()
{
  CreateDepthResourcesInternal();
  CreateSwapChainInternal();
  CreateDefaultRenderPass();
  CreateRenderFramesInternal();
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

void VulkanRenderer::DestroyTextureInternal(VulkanTexturedImageData* texturedImageData)
{
  delete texturedImageData->mSampler;
  delete texturedImageData->mImageView;
  mInternal->mAllocator->FreeAllocation(texturedImageData->mImage);
  delete texturedImageData->mImage;
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
  DestroyDefaultRenderPass();
  DestroySwapChainInternal();
  DestroyDepthResourcesInternal();

  CreateDepthResourcesInternal();
  CreateSwapChainInternal();
  CreateDefaultRenderPass();
  CreateRenderFramesInternal();
}

void VulkanRenderer::CreateSwapChainInternal()
{
  VulkanSwapChainCreationInfo swapChainInfo;
  swapChainInfo.mDevice = mInternal->mDevice;
  swapChainInfo.mPhysicalDevice = mInternal->mPhysicalDevice;
  swapChainInfo.mCommandPool = mInternal->mCommandPool;
  swapChainInfo.mGraphicsQueue = mInternal->mGraphicsQueue;
  swapChainInfo.mSurface = mInternal->mSurface;
  swapChainInfo.mExtent = Integer2(mInternal->mWidth, mInternal->mHeight);

  mInternal->mSwapChain = new VulkanSwapChain(swapChainInfo);
  mInternal->mSyncObjects.mImagesInFlight.Resize(mInternal->mSwapChain->GetCount(), VK_NULL_HANDLE);
}

void VulkanRenderer::CreateDefaultRenderPass()
{
  uint32_t frameId = mInternal->mCurrentImageIndex;
  VulkanImage* finalColorImage = mInternal->mSwapChain->GetImage(frameId);
  VulkanImage* finalDepthImage = mInternal->mDepthImage;

  VulkanImageViewInfo colorImageViewInfo;
  colorImageViewInfo.mFormat = mInternal->mSwapChain->GetImageFormat();
  colorImageViewInfo.mAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
  VulkanImageView* colorImageView = new VulkanImageView(mInternal->mDevice, finalColorImage, colorImageViewInfo);

  VulkanImageViewInfo depthImageViewInfo;
  depthImageViewInfo.mFormat = mInternal->mDepthFormat;
  depthImageViewInfo.mAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
  VulkanImageView* depthImageView = new VulkanImageView(mInternal->mDevice, finalDepthImage, depthImageViewInfo);

  mInternal->mResourcePool.Add(colorImageView);
  mInternal->mResourcePool.Add(depthImageView);

  VulkanRenderPassInfo renderPassInfo;
  renderPassInfo.mColorAttachments[0] = colorImageView;
  renderPassInfo.mColorDescriptions[0].mInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  renderPassInfo.mColorDescriptions[0].mFinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  renderPassInfo.mColorDescriptions[0].mLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  renderPassInfo.mDepthAttachment = depthImageView;
  renderPassInfo.mDepthDescription.mInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  renderPassInfo.mDepthDescription.mFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  renderPassInfo.mColorAttachmentCount = 1;
  auto& subPass = renderPassInfo.mSubPasses.PushBack();
  subPass.mColorAttachmentCount = 1;
  subPass.mColorAttachments[0] = 0;

  mInternal->mDefaultRenderPass = new VulkanRenderPass(mInternal->mDevice, renderPassInfo);
}

void VulkanRenderer::DestroyDefaultRenderPass()
{
  delete mInternal->mDefaultRenderPass;
  mInternal->mDefaultRenderPass = nullptr;
}

void VulkanRenderer::CreateRenderFramesInternal()
{
  size_t count = mInternal->mSwapChain->GetCount();
  mInternal->mRenderFrames.Resize(count);

  Array<VkCommandBuffer> commandBuffers(count);
  CreateCommandBuffers(mInternal->mDevice, mInternal->mCommandPool, commandBuffers.Data(), static_cast<uint32_t>(count));
  for(size_t i = 0; i < count; ++i)
  {
    VulkanRenderFrame& vulkanFrame = mInternal->mRenderFrames[i];
    vulkanFrame.mRenderer = this;
    VulkanCommandBufferCreationInfo commandBufferCreationInfo{mInternal->mDevice, commandBuffers[i]};
    vulkanFrame.mCommandBuffer = new VulkanCommandBuffer(commandBufferCreationInfo);
  }
}

void VulkanRenderer::DestroyRenderFramesInternal()
{
  size_t count = mInternal->mSwapChain->GetCount();
  for(VulkanRenderFrame& renderFrame : mInternal->mRenderFrames)
  {
    renderFrame.mResources.Free(*mInternal);
    renderFrame.mResources.Clear();
  }
  mInternal->mRenderFrames.Clear();
}

void VulkanRenderer::DestroySwapChainInternal()
{
  delete mInternal->mSwapChain;
  mInternal->mSwapChain = nullptr;
}

VulkanSampler* VulkanRenderer::CreateSamplerInternal(const Texture* texture)
{
  VulkanSamplerCreationInfo creationInfo;
  creationInfo.mDevice = mInternal->mDevice;
  creationInfo.mMaxLod = static_cast<float>(texture->mMipLevels);
  creationInfo.mAddressingU = ConvertSamplerAddressMode(texture->mAddressingX);
  creationInfo.mAddressingV = ConvertSamplerAddressMode(texture->mAddressingY);
  creationInfo.mMinFilter = ConvertFilterMode(texture->mMinFilter);
  creationInfo.mMagFilter = ConvertFilterMode(texture->mMagFilter);
  VulkanSampler* sampler = new VulkanSampler(creationInfo);
  return sampler;
}

VulkanImage* VulkanRenderer::CreateImageInternal(const Texture* texture)
{
  VulkanImageCreationInfo creationInfo;
  creationInfo.mDevice = mInternal->mDevice;
  creationInfo.mFormat = GetImageFormat(texture->mFormat);
  creationInfo.mWidth = static_cast<uint32_t>(texture->mSizeX);
  creationInfo.mHeight = static_cast<uint32_t>(texture->mSizeY);
  creationInfo.mMipLevels = static_cast<uint32_t>(texture->mMipLevels);
  creationInfo.mTiling = VK_IMAGE_TILING_OPTIMAL;
  creationInfo.mUsage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  creationInfo.mProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  creationInfo.mType = VK_IMAGE_TYPE_2D;
  VulkanImage* image = new VulkanImage(creationInfo);
  return image;
}

void VulkanRenderer::CreateImageMemoryInternal(const Texture* texture, VulkanImage* image, VulkanTexturedImageData& texturedImageData)
{
  VkDeviceMemory& imageMemory = texturedImageData.mImageMemory;
  VulkanMemoryAllocator& allocator = *mInternal->mAllocator;
  const float* pixels = texture->mTextureData.Data();
  VkDeviceSize imageSize = static_cast<uint32_t>(texture->mTextureData.Size());

  VulkanImageCreationInfo creationInfo = image->GetCreationInfo();
  VkImage vkImage = image->GetVulkanImage();
  
  VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  VulkanBufferCreationInfo stagingBufferCreationInfo;
  stagingBufferCreationInfo.mDevice = mInternal->mDevice;
  stagingBufferCreationInfo.mSize = imageSize;
  stagingBufferCreationInfo.mUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VulkanBuffer stagingBuffer(stagingBufferCreationInfo);

  VkDeviceMemory stagingBufferMemory = allocator.AllocateBufferMemory(&stagingBuffer, propertyFlags, true);
  vkBindBufferMemory(mInternal->mDevice, stagingBuffer.GetVulkanBuffer(), stagingBufferMemory, 0);

  {
    void* data;
    vkMapMemory(mInternal->mDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(mInternal->mDevice, stagingBufferMemory);
  }

  imageMemory = allocator.AllocateImageMemory(image, false);
  vkBindImageMemory(mInternal->mDevice, vkImage, imageMemory, 0);

  
  {
    ImageLayoutTransitionInfo transitionInfo;
    transitionInfo.mDevice = mInternal->mDevice;
    transitionInfo.mGraphicsQueue = mInternal->mGraphicsQueue;
    transitionInfo.mCommandPool = mInternal->mCommandPool;
    transitionInfo.mFormat = creationInfo.mFormat;
    transitionInfo.mImage = vkImage;
    transitionInfo.mOldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    transitionInfo.mNewLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transitionInfo.mMipLevels = creationInfo.mMipLevels;
    TransitionImageLayout(transitionInfo);
  }

  {
    ImageCopyInfo copyInfo;
    copyInfo.mDevice = mInternal->mDevice;
    copyInfo.mGraphicsQueue = mInternal->mGraphicsQueue;
    copyInfo.mCommandPool = mInternal->mCommandPool;
    copyInfo.mBuffer = stagingBuffer.GetVulkanBuffer();
    copyInfo.mWidth = creationInfo.mWidth;
    copyInfo.mHeight = creationInfo.mHeight;
    copyInfo.mImage = vkImage;
    CopyBufferToImage(copyInfo);
  }
  //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

  stagingBuffer.Free();
  allocator.FreeAllocation(stagingBufferMemory);

  {
    MipmapGenerationInfo mipGenerationInfo;
    mipGenerationInfo.mPhysicalDevice = mInternal->mPhysicalDevice;
    mipGenerationInfo.mDevice = mInternal->mDevice;
    mipGenerationInfo.mGraphicsQueue = mInternal->mGraphicsQueue;
    mipGenerationInfo.mCommandPool = mInternal->mCommandPool;
    mipGenerationInfo.mImage = vkImage;
    mipGenerationInfo.mWidth = creationInfo.mWidth;
    mipGenerationInfo.mHeight = creationInfo.mHeight;
    mipGenerationInfo.mFormat = creationInfo.mFormat;
    mipGenerationInfo.mMipLevels = creationInfo.mMipLevels;
    GenerateMipmaps(mipGenerationInfo);
  }
}

VulkanImageView* VulkanRenderer::CreateImageViewInternal(const Texture* texture, VulkanImage* image)
{
  VulkanImageViewInfo creationInfo;
  creationInfo.mFormat = GetImageFormat(texture->mFormat);
  creationInfo.mViewType = VK_IMAGE_VIEW_TYPE_2D;
  creationInfo.mAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
  creationInfo.mMipLevels = static_cast<uint32_t>(texture->mMipLevels);
  creationInfo.mComponents[0] = VK_COMPONENT_SWIZZLE_IDENTITY;
  creationInfo.mComponents[1] = VK_COMPONENT_SWIZZLE_IDENTITY;
  creationInfo.mComponents[2] = VK_COMPONENT_SWIZZLE_IDENTITY;
  creationInfo.mComponents[3] = VK_COMPONENT_SWIZZLE_IDENTITY;
  creationInfo.mAspectFlags  = creationInfo.mAspectFlags;
  creationInfo.mBaseMipLevel = 0;
  creationInfo.mBaseArrayLayer = 0;
  creationInfo.mLayerCount = 1;

  VulkanImageView* imageView = new VulkanImageView(mInternal->mDevice, image, creationInfo);
  return imageView;
}

void VulkanRenderer::CreateDepthResourcesInternal()
{
  mInternal->mDepthFormat = FindDepthFormat(mInternal->mPhysicalDevice);

  VulkanImageCreationInfo creationInfo;
  creationInfo.mDevice = mInternal->mDevice;
  creationInfo.mWidth = mInternal->mWidth;
  creationInfo.mHeight = mInternal->mHeight;
  creationInfo.mMipLevels = 1;
  creationInfo.mFormat = mInternal->mDepthFormat;
  creationInfo.mTiling = VK_IMAGE_TILING_OPTIMAL;
  creationInfo.mUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  creationInfo.mType = VK_IMAGE_TYPE_2D;
  mInternal->mDepthImage = new VulkanImage(creationInfo);

  VkImage depthImage = mInternal->mDepthImage->GetVulkanImage();
  VkDeviceMemory depthImageMemory = mInternal->mAllocator->AllocateImageMemory(mInternal->mDepthImage, false);
  vkBindImageMemory(mInternal->mDevice, depthImage, depthImageMemory, 0);

  VulkanImageViewInfo viewCreationInfo;
  viewCreationInfo.mFormat = mInternal->mDepthFormat;
  viewCreationInfo.mViewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCreationInfo.mComponents[4] = {VK_COMPONENT_SWIZZLE_IDENTITY};
  viewCreationInfo.mAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
  viewCreationInfo.mMipLevels = 1;
  mInternal->mDepthImageView = new VulkanImageView(mInternal->mDevice, mInternal->mDepthImage, viewCreationInfo);

  ImageLayoutTransitionInfo transitionInfo;
  transitionInfo.mDevice = mInternal->mDevice;
  transitionInfo.mGraphicsQueue = mInternal->mGraphicsQueue;
  transitionInfo.mCommandPool = mInternal->mCommandPool;
  transitionInfo.mFormat = mInternal->mDepthFormat;
  transitionInfo.mImage = depthImage;
  transitionInfo.mOldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  transitionInfo.mNewLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  transitionInfo.mMipLevels = 1;
  TransitionImageLayout(transitionInfo);
}

void VulkanRenderer::DestroyDepthResourcesInternal()
{
  mInternal->mAllocator->FreeAllocation(mInternal->mDepthImage);
  delete mInternal->mDepthImage;
  delete mInternal->mDepthImageView;
}
