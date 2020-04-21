#pragma once

#include "VulkanStatus.hpp"
#include "VulkanBufferCreation.hpp"
#include "VulkanStructures.hpp"

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

__declspec(noinline) inline void Cleanup(VkDevice device, VulkanImage& image)
{
  vkDestroyImageView(device, image.mImageView, nullptr);
  vkFreeMemory(device, image.mImageMemory, nullptr);
  vkDestroyImage(device, image.mImage, nullptr);
  vkDestroySampler(device, image.mSampler, nullptr);
}

inline void Cleanup(VkDevice device, ImageViewMemorySet& set)
{
  vkDestroyImageView(device, set.mImageView, nullptr);
  vkFreeMemory(device, set.mImageMemory, nullptr);
  vkDestroyImage(device, set.mImage, nullptr);
}

inline VulkanStatus CreateImage(ImageCreationInfo& info, VkImage& outImage)
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

struct ImageLayoutTransitionInfo
{
  VkDevice mDevice = VK_NULL_HANDLE;
  VkQueue mGraphicsQueue = VK_NULL_HANDLE;
  VkCommandPool mCommandPool = VK_NULL_HANDLE;
  VkImage mImage;
  VkFormat mFormat;
  VkImageLayout mOldLayout;
  VkImageLayout mNewLayout;
  uint32_t mMipLevels;
};

inline VulkanStatus ComputeTransitionStages(VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags& sourceMask, VkAccessFlags& destMask, VkPipelineStageFlags& sourceStage, VkPipelineStageFlags& destStage)
{
  if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    sourceMask = 0;
    destMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    sourceMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    destMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    sourceMask = 0;
    destMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  }
  else
    return VulkanStatus("unsupported layout transition!");
  return VulkanStatus();
}

inline bool HasStencilComponent(VkFormat format)
{
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

inline VulkanStatus TransitionImageLayout(ImageLayoutTransitionInfo& info)
{
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands(info.mDevice, info.mCommandPool);

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = info.mOldLayout;
  barrier.newLayout = info.mNewLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = info.mImage;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = info.mMipLevels;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.srcAccessMask = 0; // TODO
  barrier.dstAccessMask = 0; // TODO

  if(info.mNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    if(HasStencilComponent(info.mFormat))
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
  }
  else
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  ComputeTransitionStages(info.mOldLayout, info.mNewLayout, barrier.srcAccessMask, barrier.dstAccessMask, sourceStage, destinationStage);

  vkCmdPipelineBarrier(
    commandBuffer,
    sourceStage, destinationStage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
  );

  EndSingleTimeCommands(info.mDevice, info.mGraphicsQueue, info.mCommandPool, commandBuffer);
  return VulkanStatus();
}

struct ImageCopyInfo
{
  VkDevice mDevice = VK_NULL_HANDLE;
  VkQueue mGraphicsQueue = VK_NULL_HANDLE;
  VkCommandPool mCommandPool = VK_NULL_HANDLE;
  VkBuffer mBuffer;
  uint32_t mWidth;
  uint32_t mHeight;
  VkImage mImage;
};

inline void CopyBufferToImage(ImageCopyInfo info)
{
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands(info.mDevice, info.mCommandPool);

  VkBufferImageCopy region = {};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {
      info.mWidth,
      info.mHeight,
      1
  };
  vkCmdCopyBufferToImage(
    commandBuffer,
    info.mBuffer,
    info.mImage,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &region
  );

  EndSingleTimeCommands(info.mDevice, info.mGraphicsQueue, info.mCommandPool, commandBuffer);
}

struct MipmapGenerationInfo
{
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  VkDevice mDevice = VK_NULL_HANDLE;
  VkQueue mGraphicsQueue = VK_NULL_HANDLE;
  VkCommandPool mCommandPool = VK_NULL_HANDLE;
  VkImage mImage;
  VkFormat mFormat;
  uint32_t mWidth;
  uint32_t mHeight;
  uint32_t mMipLevels;
};

inline VulkanStatus GenerateMipmaps(MipmapGenerationInfo info)
{
  // Check if image format supports linear blitting
  VkFormatProperties formatProperties;
  vkGetPhysicalDeviceFormatProperties(info.mPhysicalDevice, info.mFormat, &formatProperties);

  if(!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    return VulkanStatus("texture image format does not support linear blitting!");

  VkCommandBuffer commandBuffer = BeginSingleTimeCommands(info.mDevice, info.mCommandPool);

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.image = info.mImage;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;

  int32_t mipWidth = info.mWidth;
  int32_t mipHeight = info.mHeight;

  for(uint32_t i = 1; i < info.mMipLevels; i++)
  {
    barrier.subresourceRange.baseMipLevel = i - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
      0, nullptr,
      0, nullptr,
      1, &barrier);

    VkImageBlit blit = {};
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
    blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.srcSubresource.mipLevel = i - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = {0, 0, 0};
    blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
    blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit.dstSubresource.mipLevel = i;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;

    vkCmdBlitImage(commandBuffer,
      info.mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      info.mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1, &blit,
      VK_FILTER_LINEAR);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
      0, nullptr,
      0, nullptr,
      1, &barrier);

    if(mipWidth > 1)
      mipWidth /= 2;
    if(mipHeight > 1)
      mipHeight /= 2;
  }

  barrier.subresourceRange.baseMipLevel = info.mMipLevels - 1;
  barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(commandBuffer,
    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
    0, nullptr,
    0, nullptr,
    1, &barrier);

  EndSingleTimeCommands(info.mDevice, info.mGraphicsQueue, info.mCommandPool, commandBuffer);
  return VulkanStatus();
}

struct TextureImageCreationInfo
{
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  VkDevice mDevice = VK_NULL_HANDLE;
  VkQueue mGraphicsQueue = VK_NULL_HANDLE;
  VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;
  VkCommandPool mCommandPool = VK_NULL_HANDLE;
  VkFormat mFormat;
  const void* mPixels = nullptr;
  uint32_t mPixelsSize = 0;
  uint32_t mWidth;
  uint32_t mHeight;
  uint32_t mMipLevels;
};

inline VulkanStatus CreateTextureImage(TextureImageCreationInfo& info, ImageViewMemorySet& imageSet)
{
  VkDeviceSize imageSize = info.mPixelsSize;
  uint32_t mipLevels = info.mMipLevels;

  if(info.mPixels == nullptr)
    return VulkanStatus("failed to load texture image!");

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  VulkanBufferCreationData vulkanData{info.mPhysicalDevice, info.mDevice, info.mGraphicsQueue, info.mCommandPool};
  CreateBuffer(vulkanData, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(info.mDevice, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, info.mPixels, static_cast<size_t>(info.mPixelsSize));
  vkUnmapMemory(info.mDevice, stagingBufferMemory);

  VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
  VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  {
    ImageCreationInfo imageInfo;
    imageInfo.mDevice = info.mDevice;
    imageInfo.mWidth = info.mWidth;
    imageInfo.mHeight = info.mHeight;
    imageInfo.mMipLevels = mipLevels;
    imageInfo.mFormat = info.mFormat;
    imageInfo.mTiling = tiling;
    imageInfo.mUsage = usage;
    imageInfo.mType = VK_IMAGE_TYPE_2D;
    CreateImage(imageInfo, imageSet.mImage);

    ImageMemoryCreationInfo memoryInfo;
    memoryInfo.mImage = imageSet.mImage;
    memoryInfo.mDevice = info.mDevice;
    memoryInfo.mPhysicalDevice = info.mPhysicalDevice;
    memoryInfo.mProperties = properties;
    CreateImageMemory(memoryInfo, imageSet.mImageMemory);

    vkBindImageMemory(info.mDevice, imageSet.mImage, imageSet.mImageMemory, 0);
  }

  {
    ImageLayoutTransitionInfo transitionInfo;
    transitionInfo.mDevice = info.mDevice;
    transitionInfo.mGraphicsQueue = info.mGraphicsQueue;
    transitionInfo.mCommandPool = info.mCommandPool;
    transitionInfo.mFormat = info.mFormat;
    transitionInfo.mImage = imageSet.mImage;
    transitionInfo.mOldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    transitionInfo.mNewLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transitionInfo.mMipLevels = mipLevels;
    TransitionImageLayout(transitionInfo);
  }
  {
    ImageCopyInfo copyInfo;
    copyInfo.mDevice = info.mDevice;
    copyInfo.mGraphicsQueue = info.mGraphicsQueue;
    copyInfo.mCommandPool = info.mCommandPool;
    copyInfo.mBuffer = stagingBuffer;
    copyInfo.mWidth = info.mWidth;
    copyInfo.mHeight = info.mHeight;
    copyInfo.mImage = imageSet.mImage;
    CopyBufferToImage(copyInfo);
  }
  //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
  
  vkDestroyBuffer(info.mDevice, stagingBuffer, nullptr);
  vkFreeMemory(info.mDevice, stagingBufferMemory, nullptr);

  {
    MipmapGenerationInfo mipGenerationInfo;
    mipGenerationInfo.mPhysicalDevice = info.mPhysicalDevice;
    mipGenerationInfo.mDevice = info.mDevice;
    mipGenerationInfo.mGraphicsQueue = info.mGraphicsQueue;
    mipGenerationInfo.mCommandPool = info.mCommandPool;
    mipGenerationInfo.mImage = imageSet.mImage;
    mipGenerationInfo.mWidth = info.mWidth;
    mipGenerationInfo.mHeight = info.mHeight;
    mipGenerationInfo.mFormat = info.mFormat;
    mipGenerationInfo.mMipLevels = mipLevels;
    GenerateMipmaps(mipGenerationInfo);
  }
  return VulkanStatus();
}

inline VulkanStatus CreateTextureImage(TextureImageCreationInfo& info, VulkanImage& vulkanImage)
{
  ImageViewMemorySet set;
  set.mImage = vulkanImage.mImage;
  set.mImageMemory = vulkanImage.mImageMemory;
  set.mImageView = vulkanImage.mImageView;
  VulkanStatus result = CreateTextureImage(info, set);
  vulkanImage.mImage = set.mImage;
  vulkanImage.mImageMemory = set.mImageMemory;
  vulkanImage.mImageView = set.mImageView;
  return result;
}
