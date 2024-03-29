#pragma once

#include "VulkanStatus.hpp"

struct SyncObjects
{
  Array<VkSemaphore> mImageAvailableSemaphores;
  Array<VkSemaphore> mRenderFinishedSemaphores;
  Array<VkFence> mInFlightFences;
  Array<VkFence> mImagesInFlight;
};

inline VulkanStatus CreateSyncObjects(VkDevice device, size_t maxFrames, SyncObjects& syncObjects)
{
  syncObjects.mImageAvailableSemaphores.Resize(maxFrames);
  syncObjects.mRenderFinishedSemaphores.Resize(maxFrames);
  syncObjects.mInFlightFences.Resize(maxFrames);

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for(size_t i = 0; i < maxFrames; i++)
  {
    bool success = true;
    success |= vkCreateSemaphore(device, &semaphoreInfo, nullptr, &syncObjects.mImageAvailableSemaphores[i]) != VK_SUCCESS;
    success |= vkCreateSemaphore(device, &semaphoreInfo, nullptr, &syncObjects.mRenderFinishedSemaphores[i]) != VK_SUCCESS;
    success |= vkCreateFence(device, &fenceInfo, nullptr, &syncObjects.mInFlightFences[i]) != VK_SUCCESS;
    if(!success)
      return VulkanStatus("failed to create semaphores!");
  }
  return VulkanStatus();
}
