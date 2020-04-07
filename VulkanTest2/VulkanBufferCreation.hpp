#pragma once

#include <vulkan/vulkan.h>

struct VulkanBufferCreationData
{
  VkPhysicalDevice mPhysicalDevice;
  VkDevice mDevice;
  VkQueue mGraphicsQueue;
  VkPipeline mGraphicsPipeline;
  VkCommandPool mCommandPool;
};

inline uint32_t FindMemoryType(VulkanBufferCreationData& vulkanData, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(vulkanData.mPhysicalDevice, &memProperties);

  for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
  {
    if((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
      return i;
  }

  throw std::runtime_error("failed to find suitable memory type!");
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

inline VkCommandBuffer BeginSingleTimeCommands(VulkanBufferCreationData& vulkanData)
{
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = vulkanData.mCommandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(vulkanData.mDevice, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

inline void EndSingleTimeCommands(VulkanBufferCreationData& vulkanData, VkCommandBuffer commandBuffer)
{
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(vulkanData.mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(vulkanData.mGraphicsQueue);

  vkFreeCommandBuffers(vulkanData.mDevice, vulkanData.mCommandPool, 1, &commandBuffer);
}

inline void CopyBuffer(VulkanBufferCreationData& vulkanData, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands(vulkanData);

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

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
