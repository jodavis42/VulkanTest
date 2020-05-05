#pragma once

struct DeviceSuitabilityData
{
  VkSurfaceKHR mSurface;
  void* mUserData;
  typedef bool(*DeviceSuitableFn)(VkPhysicalDevice physicalDevice, DeviceSuitabilityData* data);
  DeviceSuitableFn mDeviceSuitabilityFn;

  DeviceSuitabilityData() : mUserData(nullptr), mDeviceSuitabilityFn(nullptr)
  {

  }
};

struct PhysicsDeviceSelectionData
{
  VkInstance mInstance;
  DeviceSuitabilityData mSuitabilityData;
};

struct PhysicsDeviceResultData
{
  VkPhysicalDevice mPhysicalDevice;
};

inline void SelectPhysicsDevice(PhysicsDeviceSelectionData& selectionData, PhysicsDeviceResultData& resultData)
{
  resultData.mPhysicalDevice = VK_NULL_HANDLE;
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(selectionData.mInstance, &deviceCount, nullptr);

  if(deviceCount == 0)
    throw std::runtime_error("failed to find GPUs with Vulkan support!");

  Array<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(selectionData.mInstance, &deviceCount, devices.Data());

  for(const auto& device : devices)
  {
    bool isSuitable = selectionData.mSuitabilityData.mDeviceSuitabilityFn(device, &selectionData.mSuitabilityData);
    if(isSuitable)
    {
      resultData.mPhysicalDevice = device;
      break;
    }
  }

  if(resultData.mPhysicalDevice == VK_NULL_HANDLE)
    throw std::runtime_error("failed to find a suitable GPU!");
}

struct PhysicalDeviceLimits
{
  uint32_t mMaxUniformBufferRange;
  VkDeviceSize mMinUniformBufferOffsetAlignment;
};

inline void QueryPhysicalDeviceLimits(VkPhysicalDevice physicalDevice, PhysicalDeviceLimits& results)
{
  VkPhysicalDeviceProperties properties;
  vkGetPhysicalDeviceProperties(physicalDevice, &properties);

  results.mMaxUniformBufferRange = properties.limits.maxUniformBufferRange;
  results.mMinUniformBufferOffsetAlignment = properties.limits.minUniformBufferOffsetAlignment;
}
