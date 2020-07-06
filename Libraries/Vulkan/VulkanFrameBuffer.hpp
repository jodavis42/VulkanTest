#pragma once

#include "VulkanRenderPass.hpp"

//-------------------------------------------------------------------VulkanFrameBuffer
class VulkanFrameBuffer
{
public:
  VulkanFrameBuffer(VkDevice device, VulkanRenderPass& renderPass, VulkanRenderPassInfo& info, uint32_t width, uint32_t height);
  ~VulkanFrameBuffer();

  void Free();

  VkFramebuffer GetVulkanFrameBuffer() const;

private:
  VkFramebuffer mFrameBuffer = VK_NULL_HANDLE;
  VkDevice mDevice = VK_NULL_HANDLE;
};
