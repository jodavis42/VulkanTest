#pragma once

#include "VulkanStandard.hpp"

class VulkanRenderPassInfo;
class VulkanRenderPass;
struct VulkanRuntimeData;

//-------------------------------------------------------------------RenderPassCache
struct RenderPassCache
{
  RenderPassCache();
  RenderPassCache(VkDevice device);
  ~RenderPassCache();

  void Free();

private:

  VkDevice mDevice;
};
