#include "Precompiled.hpp"

#include "VulkanImage.hpp"

#include "VulkanStatus.hpp"
#include "VulkanStatus.hpp"

//-------------------------------------------------------------------VulkanImage
VulkanImage::VulkanImage(VulkanImageCreationInfo& creationInfo)
{
  VkImageCreateInfo imageInfo = {};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = creationInfo.mType;
  imageInfo.extent.width = creationInfo.mWidth;
  imageInfo.extent.height = creationInfo.mHeight;
  imageInfo.extent.depth = creationInfo.mDepth;
  imageInfo.mipLevels = creationInfo.mMipLevels;
  imageInfo.arrayLayers = 1;
  imageInfo.format = creationInfo.mFormat;
  imageInfo.tiling = creationInfo.mTiling;
  imageInfo.initialLayout = creationInfo.mInitialLayout;
  imageInfo.usage = creationInfo.mUsage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0; // Optional

  if(vkCreateImage(creationInfo.mDevice, &imageInfo, nullptr, &mImage) != VK_SUCCESS)
    VulkanStatus("failed to create image!");

  mInfo = creationInfo;
}

VulkanImage::VulkanImage(VkImage image, VulkanImageCreationInfo& creationInfo)
{
  mImage = image;
  mInfo = creationInfo;
}

VulkanImage::~VulkanImage()
{
  Free();
}

void VulkanImage::Free()
{
  if(mInfo.mDevice != VK_NULL_HANDLE)
    vkDestroyImage(mInfo.mDevice, mImage, nullptr);

  Clear();
}

void VulkanImage::Clear()
{
  mInfo = VulkanImageCreationInfo();
  mImage = VK_NULL_HANDLE;
}

uint32_t VulkanImage::GetWidth() const
{
  return mInfo.mWidth;
}

uint32_t VulkanImage::GetHeight() const
{
  return mInfo.mHeight;
}

VkFormat VulkanImage::GetInitialFormat() const
{
  return mInfo.mFormat;
}

VkImage VulkanImage::GetVulkanImage() const
{
  return mImage;
}

VulkanImageCreationInfo VulkanImage::GetCreationInfo() const
{
  return mInfo;
}
