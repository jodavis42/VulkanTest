#pragma once

#include "VulkanStatus.hpp"
#include "VulkanBufferCreation.hpp"

struct ImageViewMemorySet
{
  VkImage mImage;
  VkDeviceMemory mImageMemory;
  VkImageView mImageView;
};

struct ImageCreationInfo
{
  VkDevice mDevice = VK_NULL_HANDLE;

  uint32_t mWidth = 1;
  uint32_t mHeight = 1;
  VkFormat mFormat = VK_FORMAT_R8G8B8A8_SRGB;
  VkImageType mType = VK_IMAGE_TYPE_2D;
  VkImageUsageFlags mUsage;
  VkMemoryPropertyFlags mProperties;
  VkImageTiling mTiling = VK_IMAGE_TILING_OPTIMAL;
  uint32_t mMipLevels = 1;
};

struct ImageViewCreationInfo
{
  ImageViewCreationInfo() {}
  ImageViewCreationInfo(VkDevice device, VkImage image)
    : mDevice(device), mImage(image)
  {

  }

  // Required
  VkDevice mDevice = VK_NULL_HANDLE;
  VkImage mImage = VK_NULL_HANDLE;

  VkFormat mFormat = VK_FORMAT_R8G8B8A8_SRGB;
  VkImageViewType mViewType = VK_IMAGE_VIEW_TYPE_2D;
  VkImageAspectFlags mAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
  uint32_t mMipLevels = 1;
  uint32_t mBaseMipLevel = 0;
};

void Cleanup(VkDevice device, ImageViewMemorySet& set)
{
  vkDestroyImageView(device, set.mImageView, nullptr);
  vkFreeMemory(device, set.mImageMemory, nullptr);
  vkDestroyImage(device, set.mImage, nullptr);
}

VulkanStatus CreateImage(ImageCreationInfo& info, VkImage& outImage)
{
  VkImageCreateInfo imageInfo = {};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = info.mType;
  imageInfo.extent.width = info.mWidth;
  imageInfo.extent.height = info.mHeight;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = info.mMipLevels;
  imageInfo.arrayLayers = 1;
  imageInfo.format = info.mFormat;
  imageInfo.tiling = info.mTiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = info.mUsage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0; // Optional

  if(vkCreateImage(info.mDevice, &imageInfo, nullptr, &outImage) != VK_SUCCESS)
    return VulkanStatus("failed to create image!");

  return VulkanStatus();
}

struct ImageMemoryCreationInfo
{
  VkDevice mDevice;
  VkPhysicalDevice mPhysicalDevice;
  VkImage mImage;
  VkMemoryPropertyFlags mProperties;
};

inline VulkanStatus CreateImageMemory(ImageMemoryCreationInfo& info, VkDeviceMemory& outImageMemory)
{
  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(info.mDevice, info.mImage, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  FindMemoryType(info.mPhysicalDevice, memRequirements.memoryTypeBits, info.mProperties, allocInfo.memoryTypeIndex);

  if(vkAllocateMemory(info.mDevice, &allocInfo, nullptr, &outImageMemory) != VK_SUCCESS)
    return VulkanStatus("failed to allocate image memory!");
  return VulkanStatus();
}

inline VulkanStatus CreateImageView(ImageViewCreationInfo& creationInfo, VkImageView& outImageView)
{
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = creationInfo.mImage;
  viewInfo.viewType = creationInfo.mViewType;
  viewInfo.format = creationInfo.mFormat;
  viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.subresourceRange.aspectMask = creationInfo.mAspectFlags;
  viewInfo.subresourceRange.baseMipLevel = creationInfo.mBaseMipLevel;
  viewInfo.subresourceRange.levelCount = creationInfo.mMipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if(vkCreateImageView(creationInfo.mDevice, &viewInfo, nullptr, &outImageView) != VK_SUCCESS)
    return VulkanStatus("failed to create texture image view!");

  return VulkanStatus();
}

struct SamplerCreationInfo
{
  VkDevice mDevice = VK_NULL_HANDLE;

  VkFilter mMinFilter = VK_FILTER_LINEAR;
  VkFilter mMagFilter = VK_FILTER_LINEAR;
  VkSamplerAddressMode mAddressingU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  VkSamplerAddressMode mAddressingV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  VkSamplerAddressMode mAddressingW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  VkBool32 mAnistropyEnabled = VK_TRUE;
  float mMaxAnistropy = 16;
  VkCompareOp mCompareOp = VK_COMPARE_OP_ALWAYS;
  VkBool32 mCompareMode = VK_FALSE;
  VkSamplerMipmapMode mMipMapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  float mMipLodBias = 0.0f;
  float mMinLod = 0.0f;
  float mMaxLod = 1.0f;
};

inline VulkanStatus CreateTextureSampler(SamplerCreationInfo& info, VkSampler& sampler)
{
  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.minFilter = info.mMinFilter;
  samplerInfo.magFilter = info.mMagFilter;
  samplerInfo.addressModeU = info.mAddressingU;
  samplerInfo.addressModeV = info.mAddressingV;
  samplerInfo.addressModeW = info.mAddressingW;
  samplerInfo.anisotropyEnable = info.mAnistropyEnabled;
  samplerInfo.maxAnisotropy = info.mMaxAnistropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = info.mCompareMode;
  samplerInfo.compareOp = info.mCompareOp;
  samplerInfo.mipmapMode = info.mMipMapMode;
  samplerInfo.mipLodBias = info.mMipLodBias;
  samplerInfo.minLod = info.mMinLod;
  samplerInfo.maxLod = info.mMaxLod;

  if(vkCreateSampler(info.mDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
    return VulkanStatus("failed to create texture sampler!");
  return VulkanStatus();
}
