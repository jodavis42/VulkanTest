#include "Precompiled.hpp"

#include "VulkanFrameBuffer.hpp"
#include "VulkanStatus.hpp"
#include "VulkanImageView.hpp"

//-------------------------------------------------------------------VulkanFrameBuffer
VulkanFrameBuffer::VulkanFrameBuffer(VkDevice device, VulkanRenderPass& renderPass, VulkanRenderPassInfo& info, uint32_t width, uint32_t height)
{
  mDevice = device;

  uint32_t attachmentCount = info.mColorAttachmentCount + 1;
  VkImageView attachments[9] = {};
  for(size_t i = 0; i < info.mColorAttachmentCount; ++i)
    attachments[i] = info.mColorAttachments[i]->GetVulkanImageView();
  attachments[info.mColorAttachmentCount] = info.mDepthAttachment->GetVulkanImageView();

  uint32_t layerCount = info.mNumLayers > 1 ? (info.mBaseLayer + info.mNumLayers) : 1;

  VkFramebufferCreateInfo framebufferInfo = {};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = renderPass.GetVulkanRenderPass();
  framebufferInfo.attachmentCount = attachmentCount;
  framebufferInfo.pAttachments = attachments;
  framebufferInfo.width = width;
  framebufferInfo.height = height;
  framebufferInfo.layers = layerCount;

  if(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &mFrameBuffer) != VK_SUCCESS)
    VulkanStatus("failed to create render pass!");
}

VulkanFrameBuffer::~VulkanFrameBuffer()
{
  Free();
}

void VulkanFrameBuffer::Free()
{
  vkDestroyFramebuffer(mDevice, mFrameBuffer, nullptr);
  mFrameBuffer = VK_NULL_HANDLE;
}

VkFramebuffer VulkanFrameBuffer::GetVulkanFrameBuffer() const
{
  return mFrameBuffer;
}
