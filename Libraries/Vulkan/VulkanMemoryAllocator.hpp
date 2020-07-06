#pragma once

#include "VulkanRenderPass.hpp"

class VulkanImage;
class VulkanBuffer;

//-------------------------------------------------------------------VulkanMemoryAllocatorCreationInfo
struct VulkanMemoryAllocatorCreationInfo
{
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  VkDevice mDevice = VK_NULL_HANDLE;
  VkQueue mGraphicsQueue = VK_NULL_HANDLE;
  VkCommandPool mCommandPool = VK_NULL_HANDLE;
};

//-------------------------------------------------------------------VulkanMemoryAllocator
class VulkanMemoryAllocator
{
public:
  VulkanMemoryAllocator(VulkanMemoryAllocatorCreationInfo& creationInfo);
  ~VulkanMemoryAllocator();

  VkDeviceMemory AllocateImageMemory(const VulkanImage* image, bool transient);
  VkDeviceMemory AllocateBufferMemory(const VulkanBuffer* buffer, VkMemoryPropertyFlags properties, bool transient);

  void FreeAllocation(void* key);
  void FreeAllocation(VkDeviceMemory memory);
  void FreeAllAllocations();

  VkPhysicalDevice mPhysicalDevice;
  VkDevice mDevice = VK_NULL_HANDLE;
  VkQueue mGraphicsQueue = VK_NULL_HANDLE;
  VkCommandPool mCommandPool = VK_NULL_HANDLE;
  HashMap<void*, VkDeviceMemory> mAllocations;
};
