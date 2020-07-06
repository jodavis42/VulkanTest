#include "Precompiled.hpp"

#include "VulkanSwapChain.hpp"

//-------------------------------------------------------------------VulkanSwapChain
VulkanSwapChain::VulkanSwapChain(VulkanSwapChainCreationInfo& creationInfo)
{
  mDevice = creationInfo.mDevice;

  SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(creationInfo.mPhysicalDevice, creationInfo.mSurface);

  VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = ChooseSwapExtent(creationInfo, swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    imageCount = swapChainSupport.capabilities.maxImageCount;

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = creationInfo.mSurface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

  QueueFamilyIndices indices = FindQueueFamilies(creationInfo.mPhysicalDevice, creationInfo.mSurface);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if(indices.graphicsFamily != indices.presentFamily)
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  }
  else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0; // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
  }
  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if(vkCreateSwapchainKHR(creationInfo.mDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
    VulkanStatus("failed to create swap chain!");

  vkGetSwapchainImagesKHR(creationInfo.mDevice, mSwapChain, &imageCount, nullptr);
  Array<VkImage> images;
  images.Resize(imageCount);
  vkGetSwapchainImagesKHR(creationInfo.mDevice, mSwapChain, &imageCount, images.Data());

  for(size_t i = 0; i < imageCount; ++i)
  {
    ImageLayoutTransitionInfo transitionInfo;
    transitionInfo.mCommandPool = creationInfo.mCommandPool;
    transitionInfo.mGraphicsQueue = creationInfo.mGraphicsQueue;
    transitionInfo.mDevice = creationInfo.mDevice;
    transitionInfo.mImage = images[i];
    transitionInfo.mNewLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transitionInfo.mOldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    transitionInfo.mMipLevels = 1;
    TransitionImageLayout(transitionInfo);
  }

  mImages.Resize(imageCount);
  for(size_t i = 0; i < imageCount; ++i)
  {
    VulkanImageCreationInfo imageCreateInfo;
    imageCreateInfo.mDevice = creationInfo.mDevice;
    imageCreateInfo.mType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.mFormat = surfaceFormat.format;
    imageCreateInfo.mUsage = createInfo.imageUsage;
    imageCreateInfo.mProperties = 0;
    imageCreateInfo.mTiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.mInitialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    imageCreateInfo.mWidth = extent.width;
    imageCreateInfo.mHeight = extent.height;
    imageCreateInfo.mMipLevels = creationInfo.mMipLevels;

    mImages[i] = new VulkanImage(images[i], imageCreateInfo);
  }

  mImageFormat = surfaceFormat.format;
  mExtent = extent;
}

VulkanSwapChain::~VulkanSwapChain()
{
  Free();
}

VkFormat VulkanSwapChain::GetImageFormat() const
{
  return mImageFormat;
}

VkExtent2D VulkanSwapChain::GetExtent() const
{
  return mExtent;
}

VulkanImage* VulkanSwapChain::GetImage(size_t imageIndex) const
{
  ErrorIf(imageIndex >= mImages.Size(), "Invalid index");
  return mImages[imageIndex];
}

uint32_t VulkanSwapChain::GetCount() const
{
  return static_cast<uint32_t>(mImages.Size());
}

VkSwapchainKHR VulkanSwapChain::GetVulkanSwapChain() const
{
  return mSwapChain;
}

void VulkanSwapChain::Free()
{
  for(size_t i = 0; i < mImages.Size(); ++i)
  {
    mImages[i]->Clear();
    delete mImages[i];
  }
  mImages.Clear();

  if(mSwapChain != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
  mSwapChain = VK_NULL_HANDLE;
}

VkSurfaceFormatKHR VulkanSwapChain::ChooseSwapSurfaceFormat(const Array<VkSurfaceFormatKHR>& availableFormats)
{
  for(const auto& availableFormat : availableFormats)
  {
    if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return availableFormat;
  }

  return availableFormats[0];
}

VkPresentModeKHR VulkanSwapChain::ChooseSwapPresentMode(const Array<VkPresentModeKHR>& availablePresentModes)
{
  for(const auto& availablePresentMode : availablePresentModes)
  {
    if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      return availablePresentMode;
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChain::ChooseSwapExtent(VulkanSwapChainCreationInfo& info, const VkSurfaceCapabilitiesKHR& capabilities)
{
  if(capabilities.currentExtent.width != UINT32_MAX)
  {
    return capabilities.currentExtent;
  }
  else
  {
    VkExtent2D actualExtent;
    actualExtent.width = info.mExtent.x;
    actualExtent.height = info.mExtent.y;
    return actualExtent;
  }
}
