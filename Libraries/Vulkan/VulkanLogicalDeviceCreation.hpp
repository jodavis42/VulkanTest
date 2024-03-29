#pragma once

#include "VulkanDeviceQueries.hpp"
#include <vector>
#include <set>

struct LogicalDeviceCreationData
{
  VkPhysicalDevice mPhysicalDevice;
  VkSurfaceKHR mSurface;
  Array<const char*> mDeviceExtensions;
};

struct LogicalDeviceResultData
{
  VkDevice mDevice;
  VkQueue mGraphicsQueue;
  VkQueue mPresentQueue;
};

inline void CreateLogicalDevice(LogicalDeviceCreationData& creationData, LogicalDeviceResultData& resultData)
{
  QueueFamilyIndices indices = FindQueueFamilies(creationData.mPhysicalDevice, creationData.mSurface);

  Array<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  float queuePriority = 1.0f;
  for(uint32_t queueFamily : uniqueQueueFamilies)
  {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.PushBack(queueCreateInfo);
  }

  VkDeviceQueueCreateInfo queueCreateInfo = {};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
  queueCreateInfo.queueCount = 1;

  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.Size());
  createInfo.pQueueCreateInfos = queueCreateInfos.Data();

  createInfo.pEnabledFeatures = &deviceFeatures;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(creationData.mDeviceExtensions.Size());
  createInfo.ppEnabledExtensionNames = creationData.mDeviceExtensions.Data();

  if(enableValidationLayers)
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.Size());
    createInfo.ppEnabledLayerNames = validationLayers.Data();
  }
  else
  {
    createInfo.enabledLayerCount = 0;
  }

  if(vkCreateDevice(creationData.mPhysicalDevice, &createInfo, nullptr, &resultData.mDevice) != VK_SUCCESS)
    throw std::runtime_error("failed to create logical device!");

  vkGetDeviceQueue(resultData.mDevice, indices.graphicsFamily.value(), 0, &resultData.mGraphicsQueue);
  vkGetDeviceQueue(resultData.mDevice, indices.presentFamily.value(), 0, &resultData.mPresentQueue);
}
