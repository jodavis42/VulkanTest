#pragma once

#include <vulkan/vulkan.h>
#include "VulkanStatus.hpp"
#include "VulkanPhysicsDeviceSelection.hpp"

struct VulkanBufferCreationData
{
  VkPhysicalDevice mPhysicalDevice;
  VkDevice mDevice;
  VkQueue mGraphicsQueue;
  //VkPipeline mGraphicsPipeline;
  VkCommandPool mCommandPool;
};

inline VulkanStatus FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t& outMemoryType)
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
  {
    if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
    {
      outMemoryType = i;
      return VulkanStatus();
    }
  }

  return VulkanStatus("failed to find suitable memory type!");
}

inline uint32_t FindMemoryType(VulkanBufferCreationData& vulkanData, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
  uint32_t result = 0;
  FindMemoryType(vulkanData.mPhysicalDevice, typeFilter, properties, result);
  return result;
}

inline void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    throw std::runtime_error("failed to create vertex buffer!");

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties, allocInfo.memoryTypeIndex);

  if(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate vertex buffer memory!");

  vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

inline void CreateBuffer(VulkanBufferCreationData& vulkanData, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if(vkCreateBuffer(vulkanData.mDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    throw std::runtime_error("failed to create vertex buffer!");

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(vulkanData.mDevice, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = FindMemoryType(vulkanData, memRequirements.memoryTypeBits, properties);

  if(vkAllocateMemory(vulkanData.mDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate vertex buffer memory!");

  vkBindBufferMemory(vulkanData.mDevice, buffer, bufferMemory, 0);
}

inline VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool)
{
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

inline void EndSingleTimeCommands(VkDevice device, VkQueue graphicsQueue, VkCommandPool commandPool, VkCommandBuffer commandBuffer)
{
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(graphicsQueue);

  vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

inline VkCommandBuffer BeginSingleTimeCommands(VulkanBufferCreationData& vulkanData)
{
  return BeginSingleTimeCommands(vulkanData.mDevice, vulkanData.mCommandPool);
}

inline void EndSingleTimeCommands(VulkanBufferCreationData& vulkanData, VkCommandBuffer commandBuffer)
{
  EndSingleTimeCommands(vulkanData.mDevice, vulkanData.mGraphicsQueue, vulkanData.mCommandPool, commandBuffer);
}

inline void CopyBuffer(VulkanBufferCreationData& vulkanData, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands(vulkanData);

  VkBufferCopy copyRegion = {};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  EndSingleTimeCommands(vulkanData, commandBuffer);
}

inline void CreateBuffer(VulkanBufferCreationData& vulkanData, VkBufferUsageFlags bufferUsage, VkBuffer& buffer, VkDeviceMemory& bufferMemory, const void* initialData, size_t dataSize)
{
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(vulkanData, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(vulkanData.mDevice, stagingBufferMemory, 0, dataSize, 0, &data);
  memcpy(data, initialData, dataSize);
  vkUnmapMemory(vulkanData.mDevice, stagingBufferMemory);

  CreateBuffer(vulkanData, dataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);

  CopyBuffer(vulkanData, stagingBuffer, buffer, dataSize);

  vkDestroyBuffer(vulkanData.mDevice, stagingBuffer, nullptr);
  vkFreeMemory(vulkanData.mDevice, stagingBufferMemory, nullptr);
}

inline size_t AlignUniformBufferOffset(PhysicalDeviceLimits& deviceLimits, size_t offset)
{
  size_t alignment = deviceLimits.mMinUniformBufferOffsetAlignment;
  size_t extra = offset % alignment;
  size_t result = offset / alignment + extra != 0 ? alignment : 0;
  return result;
}