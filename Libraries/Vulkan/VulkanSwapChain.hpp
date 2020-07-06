#pragma once

#include "Math.hpp"
#include "VulkanStatus.hpp"
#include "VulkanDeviceQueries.hpp"
#include "VulkanImages.hpp"
#include "VulkanImage.hpp"

//-------------------------------------------------------------------VulkanSwapChainCreationInfo
struct VulkanSwapChainCreationInfo
{
  VkDevice mDevice = VK_NULL_HANDLE;
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  VkQueue mGraphicsQueue = VK_NULL_HANDLE;
  VkCommandPool mCommandPool = VK_NULL_HANDLE;

  VkSurfaceKHR mSurface = VK_NULL_HANDLE;
  VkFormat mFormat = VK_FORMAT_UNDEFINED;
  uint32_t mMipLevels = 1;

  Integer2 mExtent;
};

//-------------------------------------------------------------------VulkanSwapChain
class VulkanSwapChain
{
public:
  VulkanSwapChain(VulkanSwapChainCreationInfo& creationInfo);
  ~VulkanSwapChain();

  VkFormat GetImageFormat() const;
  VkExtent2D GetExtent() const;
  
  VulkanImage* GetImage(size_t imageIndex) const;
  uint32_t GetCount() const;

  VkSwapchainKHR GetVulkanSwapChain() const;

  void Free();

private:

  VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const Array<VkSurfaceFormatKHR>& availableFormats);
  VkPresentModeKHR ChooseSwapPresentMode(const Array<VkPresentModeKHR>& availablePresentModes);
  VkExtent2D ChooseSwapExtent(VulkanSwapChainCreationInfo& info, const VkSurfaceCapabilitiesKHR& capabilities);

  Array<VulkanImage*> mImages;
  VkDevice mDevice = VK_NULL_HANDLE;
  VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
  VkFormat mImageFormat = VK_FORMAT_UNDEFINED;
  VkExtent2D mExtent;
};
