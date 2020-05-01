#pragma once

#include "Math.hpp"
#include "VulkanStatus.hpp"
#include "VulkanDeviceQueries.hpp"
#include "VulkanImages.hpp"

struct SwapChainCreationInfo
{
  VkDevice mDevice;
  VkPhysicalDevice mPhysicalDevice;
  VkSurfaceKHR mSurface;
  VkFormat mFormat;
  uint32_t mMipLevels = 1;

  void* mUserData = nullptr;
  typedef void(*FrameBufferSizeQueryFn)(uint32_t& width, uint32_t& height, void* userData);
  FrameBufferSizeQueryFn mQueryFn = nullptr;
  Integer2 mExtent;
};

struct SwapChainResultInfo
{
  uint32_t GetCount() const
  {
    return static_cast<uint32_t>(mImages.size());
  }

  VkSwapchainKHR mSwapChain;
  std::vector<VkImage> mImages;
  VkFormat mImageFormat;
  VkExtent2D mExtent;
  std::vector<VkImageView> mImageViews;
};

struct SwapChainData : public SwapChainResultInfo
{

};

inline VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
  for(const auto& availableFormat : availableFormats)
  {
    if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return availableFormat;
  }

  return availableFormats[0];
}

inline VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
  for(const auto& availablePresentMode : availablePresentModes)
  {
    if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      return availablePresentMode;
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

inline VkExtent2D ChooseSwapExtent(SwapChainCreationInfo& info, const VkSurfaceCapabilitiesKHR& capabilities)
{
  if(capabilities.currentExtent.width != UINT32_MAX)
  {
    return capabilities.currentExtent;
  }
  else
  {
    VkExtent2D actualExtent;
    if(info.mQueryFn != nullptr)
      info.mQueryFn(actualExtent.width, actualExtent.height, info.mUserData);
    else
    {
      actualExtent.width = info.mExtent.x;
      actualExtent.height = info.mExtent.y;
    }
    return actualExtent;
  }
}

inline VulkanStatus CreateSwapChain(SwapChainCreationInfo& info, SwapChainResultInfo& result)
{
  SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(info.mPhysicalDevice, info.mSurface);

  VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = ChooseSwapExtent(info, swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    imageCount = swapChainSupport.capabilities.maxImageCount;

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = info.mSurface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = FindQueueFamilies(info.mPhysicalDevice, info.mSurface);
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

  if(vkCreateSwapchainKHR(info.mDevice, &createInfo, nullptr, &result.mSwapChain) != VK_SUCCESS)
    return VulkanStatus("failed to create swap chain!");

  vkGetSwapchainImagesKHR(info.mDevice, result.mSwapChain, &imageCount, nullptr);
  result.mImages.resize(imageCount);
  vkGetSwapchainImagesKHR(info.mDevice, result.mSwapChain, &imageCount, result.mImages.data());

  result.mImageFormat = surfaceFormat.format;
  result.mExtent = extent;
  return VulkanStatus();
}

struct SwapChainImageViewCreationInfo
{
  VkDevice mDevice;
  VkFormat mFormat;
  uint32_t mMipLevels;
  std::vector<VkImage> mImages;
};

struct SwapChainImageViewResultInfo
{
  std::vector<VkImageView> mImageViews;
};

inline void CreateSwapChainImageView(VkDevice device, VkFormat format, uint32_t mipLevels, VkImage image, VkImageView& outImageView)
{
  ImageViewCreationInfo info(device, image);
  info.mFormat = format;
  info.mViewType = VK_IMAGE_VIEW_TYPE_2D;
  info.mAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
  info.mMipLevels = mipLevels;
  VulkanStatus status = CreateImageView(info, outImageView);
}

inline void CreateSwapChainImageViews(SwapChainImageViewCreationInfo& info, SwapChainImageViewResultInfo& result)
{
  size_t count = info.mImages.size();
  result.mImageViews.resize(count);

  for(size_t i = 0; i < count; i++)
  {
    CreateSwapChainImageView(info.mDevice, info.mFormat, info.mMipLevels, info.mImages[i], result.mImageViews[i]);
  }
}

inline VulkanStatus CreateSwapChainAndViews(SwapChainCreationInfo& info, SwapChainData& result)
{
  CreateSwapChain(info, result);

  SwapChainImageViewResultInfo viewResultInfo;
  SwapChainImageViewCreationInfo viewCreationInfo;
  viewCreationInfo.mDevice = info.mDevice;
  viewCreationInfo.mFormat = result.mImageFormat;
  viewCreationInfo.mImages = result.mImages;
  viewCreationInfo.mMipLevels = info.mMipLevels;
  CreateSwapChainImageViews(viewCreationInfo, viewResultInfo);
  result.mImageViews = viewResultInfo.mImageViews;
  return VulkanStatus();
}
