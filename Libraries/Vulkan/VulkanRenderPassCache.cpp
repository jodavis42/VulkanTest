#include "Precompiled.hpp"

#include "VulkanRenderPassCache.hpp"

#include "VulkanInitialization.hpp"
#include "VulkanImage.hpp"
#include "VulkanImageView.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanStatus.hpp"
#include "VulkanStructures.hpp"
#include "VulkanCommandBuffer.hpp"

//-------------------------------------------------------------------RenderPassCache
RenderPassCache::RenderPassCache()
{
}

RenderPassCache::RenderPassCache(VkDevice device)
  : mDevice(device)
{
}

RenderPassCache::~RenderPassCache()
{
  Free();
}

void RenderPassCache::Free()
{
}
