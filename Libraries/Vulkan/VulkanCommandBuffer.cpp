#include "Precompiled.hpp"

#include "VulkanCommandBuffer.hpp"

#include "VulkanStatus.hpp"
#include "VulkanImage.hpp"
#include "VulkanImageView.hpp"

VulkanStatus CreateCommandBuffers(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* resultBuffers, uint32_t resultBuffersCount)
{
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = resultBuffersCount;

  VulkanStatus result;
  if(vkAllocateCommandBuffers(device, &allocInfo, resultBuffers) != VK_SUCCESS)
    result.MarkFailed("failed to allocate command buffers!");
  return result;

}
//-------------------------------------------------------------------VulkanCommandBuffer
VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBufferCreationInfo& creationInfo)
{
  mDevice = creationInfo.mDevice;
  mCommandBuffer = creationInfo.mCommandBuffer;
}

void VulkanCommandBuffer::Begin()
{
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0; // Optional
  beginInfo.pInheritanceInfo = nullptr; // Optional

  VulkanStatus result;
  if(vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS)
    VulkanStatus("failed to begin recording command buffer!");
}

void VulkanCommandBuffer::End()
{
  if(vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS)
    VulkanStatus("failed to record command buffer!");
}

void VulkanCommandBuffer::BeginRenderPass()
{

}

void VulkanCommandBuffer::EndRenderPass()
{
  vkCmdEndRenderPass(mCommandBuffer);
}

void VulkanCommandBuffer::ClearColorImage(VulkanImage& image, const VkClearColorValue& clearColor, VkImageLayout imageLayout, VkImageAspectFlags aspectFlags)
{
  VkImageSubresourceRange subresourceRange = {};
  subresourceRange.aspectMask = aspectFlags;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.baseArrayLayer = 0;
  subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
  subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

  vkCmdClearColorImage(mCommandBuffer, image.GetVulkanImage(), imageLayout, &clearColor, 1, &subresourceRange);
}

void VulkanCommandBuffer::ClearDepthImage(VulkanImage& image, const VkClearDepthStencilValue& clearDepth, VkImageLayout imageLayout, VkImageAspectFlags aspectFlags)
{
  VkImageSubresourceRange subresourceRange = {};
  subresourceRange.aspectMask = aspectFlags;
  subresourceRange.baseMipLevel = 0;
  subresourceRange.baseArrayLayer = 0;
  subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
  subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

  vkCmdClearDepthStencilImage(mCommandBuffer, image.GetVulkanImage(), imageLayout, &clearDepth, 1, &subresourceRange);
}

void VulkanCommandBuffer::ImageBarrier(VulkanImageView& imageView,
  VkImageLayout oldLayout, VkImageLayout newLayout,
  VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
  VkPipelineStageFlags srcStageFlags, VkPipelineStageFlags dstStageFlags)
{
  ImageBarrier(*imageView.GetImage(), imageView.GetCreationInfo().mAspectFlags, oldLayout, newLayout, srcAccessMask, dstAccessMask, srcStageFlags, dstStageFlags);
}

void VulkanCommandBuffer::ImageBarrier(VulkanImage& image, VkImageAspectFlags aspectFlags,
  VkImageLayout oldLayout, VkImageLayout newLayout,
  VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
  VkPipelineStageFlags srcStageFlags, VkPipelineStageFlags dstStageFlags)
{
  VkImageMemoryBarrier imageBarrier{};
  imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageBarrier.srcAccessMask = srcAccessMask;
  imageBarrier.dstAccessMask = dstAccessMask;
  imageBarrier.oldLayout = oldLayout;
  imageBarrier.newLayout = newLayout;
  imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageBarrier.image = image.GetVulkanImage();
  imageBarrier.subresourceRange.aspectMask = aspectFlags;
  imageBarrier.subresourceRange.baseMipLevel = 0;
  imageBarrier.subresourceRange.baseArrayLayer = 0;
  imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
  imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

  vkCmdPipelineBarrier(mCommandBuffer, srcStageFlags, dstStageFlags, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
}

VkCommandBuffer VulkanCommandBuffer::GetVulkanCommandBuffer() const
{
  return mCommandBuffer;
}

