#include "Precompiled.hpp"

#include "VulkanMemoryAllocator.hpp"

#include "VulkanStatus.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanImage.hpp"
#include "VulkanImages.hpp"
#include "VulkanBufferCreation.hpp"

//-------------------------------------------------------------------VulkanMemoryAllocator
VulkanMemoryAllocator::VulkanMemoryAllocator(VulkanMemoryAllocatorCreationInfo& creationInfo)
{
  mPhysicalDevice = creationInfo.mPhysicalDevice;
  mDevice = creationInfo.mDevice;
  mGraphicsQueue = creationInfo.mGraphicsQueue;
  mCommandPool = creationInfo.mCommandPool;
}

VulkanMemoryAllocator::~VulkanMemoryAllocator()
{
  FreeAllAllocations();
}

VkDeviceMemory VulkanMemoryAllocator::AllocateImageMemory(const VulkanImage* image, bool transient)
{
  VulkanImageCreationInfo imageInfo = image->GetCreationInfo();

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(mDevice, image->GetVulkanImage(), &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  FindMemoryType(mPhysicalDevice, memRequirements.memoryTypeBits, imageInfo.mProperties, allocInfo.memoryTypeIndex);

  VkDeviceMemory imageMemory = VK_NULL_HANDLE;
  if(vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    VulkanStatus("failed to allocate image memory!");

  if(!transient)
    mAllocations[(void*)image] = imageMemory;

  return imageMemory;
}

VkDeviceMemory VulkanMemoryAllocator::AllocateBufferMemory(const VulkanBuffer* buffer, VkMemoryPropertyFlags properties, bool transient)
{
  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(mDevice, buffer->GetVulkanBuffer(), &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  FindMemoryType(mPhysicalDevice, memRequirements.memoryTypeBits, properties, allocInfo.memoryTypeIndex);

  VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
  if(vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    VulkanStatus("failed to allocate vertex buffer memory!");

  if(!transient)
    mAllocations[(void*)buffer] = bufferMemory;

  return bufferMemory;
}

void VulkanMemoryAllocator::FreeAllocation(void* key)
{
  VkDeviceMemory* allocation = mAllocations.FindPointer(key);
  if(allocation == nullptr)
    return;

  FreeAllocation(*allocation);
  mAllocations.Erase(key);
}

void VulkanMemoryAllocator::FreeAllocation(VkDeviceMemory memory)
{
  vkFreeMemory(mDevice, memory, nullptr);
}

void VulkanMemoryAllocator::FreeAllAllocations()
{
  for(VkDeviceMemory& allocation : mAllocations.Values())
    FreeAllocation(allocation);
  mAllocations.Clear();
}
