#include "Precompiled.hpp"

#include "VulkanBuffer.hpp"

#include "VulkanStatus.hpp"

//-------------------------------------------------------------------VulkanBuffer
VulkanBuffer::VulkanBuffer(const VulkanBufferCreationInfo& creationInfo)
{
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = creationInfo.mSize;
  bufferInfo.usage = creationInfo.mUsage;
  bufferInfo.flags = creationInfo.mCreateFlags;
  bufferInfo.sharingMode = creationInfo.mSharingMode;

  if(vkCreateBuffer(creationInfo.mDevice, &bufferInfo, nullptr, &mBuffer) != VK_SUCCESS)
    VulkanStatus("failed to create vertex buffer!");

  mInfo = creationInfo;
}

VulkanBuffer::~VulkanBuffer()
{
  Free();
}

void VulkanBuffer::Free()
{
  if(mInfo.mDevice != VK_NULL_HANDLE)
    vkDestroyBuffer(mInfo.mDevice, mBuffer, nullptr);

  mBuffer = VK_NULL_HANDLE;
  mInfo = VulkanBufferCreationInfo();
}

VkBuffer VulkanBuffer::GetVulkanBuffer() const
{
  return mBuffer;
}

VulkanBufferCreationInfo VulkanBuffer::GetInfo() const
{
  return mInfo;
}
