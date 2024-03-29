#pragma once

#include <vulkan/vulkan.h>
#include "VulkanStatus.hpp"

struct SurfaceCreationDelegate
{
  typedef VulkanStatus(*SurfaceCreationCallbackFn)(VkInstance instance, void* userData, VkSurfaceKHR& outSurface);
  SurfaceCreationCallbackFn mCallbackFn = nullptr;
  void* mUserData = nullptr;
};

struct VulkanInitializationData
{
  size_t mWidth;
  size_t mHeight;
  SurfaceCreationDelegate mSurfaceCreationCallback;
};

constexpr const char* TransformsBufferName = "Transforms";
constexpr const char* MaterialBufferName = "Material";
constexpr const char* GlobalsBufferName = "Globals";
