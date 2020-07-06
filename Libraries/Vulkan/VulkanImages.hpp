#pragma once

#include "VulkanStatus.hpp"
#include "VulkanBufferCreation.hpp"
#include "VulkanStructures.hpp"

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
  else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
  {
    sourceMask = 0;
    destMask = 0;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
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

