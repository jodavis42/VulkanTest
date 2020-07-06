#pragma once

#include "VulkanStandard.hpp"
#include "VulkanStatus.hpp"

class VulkanImage;
class VulkanImageView;

VulkanStatus CreateCommandBuffers(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* resultBuffers, uint32_t resultBuffersCount);

//-------------------------------------------------------------------VulkanCommandBufferCreationInfo
struct VulkanCommandBufferCreationInfo
{
  VkDevice mDevice = VK_NULL_HANDLE;
  VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
};

//-------------------------------------------------------------------VulkanCommandBuffer
class VulkanCommandBuffer
{
public:
  VulkanCommandBuffer(VulkanCommandBufferCreationInfo& creationInfo);

  void Begin();
  void End();

  void BeginRenderPass();
  void EndRenderPass();

  void ClearColorImage(VulkanImage& image, const VkClearColorValue& clearColor, VkImageLayout imageLayout, VkImageAspectFlags aspectFlags);
  void ClearDepthImage(VulkanImage& image, const VkClearDepthStencilValue& clearDepth, VkImageLayout imageLayout, VkImageAspectFlags aspectFlags);

  void VulkanCommandBuffer::ImageBarrier(VulkanImageView& imageView,
    VkImageLayout oldLayout, VkImageLayout newLayout,
    VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
    VkPipelineStageFlags srcStageFlags, VkPipelineStageFlags dstStageFlags);
  void VulkanCommandBuffer::ImageBarrier(VulkanImage& image, VkImageAspectFlags aspectFlags,
    VkImageLayout oldLayout, VkImageLayout newLayout,
    VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
    VkPipelineStageFlags srcStageFlags, VkPipelineStageFlags dstStageFlags);

  VkCommandBuffer GetVulkanCommandBuffer() const;

private:
  
  VkDevice mDevice = VK_NULL_HANDLE;
  VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
};

