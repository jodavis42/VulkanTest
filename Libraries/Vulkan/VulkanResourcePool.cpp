#include "Precompiled.hpp"

#include "VulkanResourcePool.hpp"

#include "VulkanImageView.hpp"
#include "VulkanFrameBuffer.hpp"
#include "VulkanRenderPass.hpp"

//-------------------------------------------------------------------VulkanResourcePool
void VulkanResourcePool::Add(VulkanImageView* imageView)
{
  mVulkanImageViews.PushBack(imageView);
}

void VulkanResourcePool::Add(VulkanRenderPass* renderPass)
{
  mRenderPasses.PushBack(renderPass);
}

void VulkanResourcePool::Add(VulkanFrameBuffer* frameBuffer)
{
  mVulkanFrameBuffers.PushBack(frameBuffer);
}

void VulkanResourcePool::Free(VulkanRuntimeData& runtimeData)
{
  for(size_t i = 0; i < mVulkanImageViews.Size(); ++i)
  {
    delete mVulkanImageViews[i];
  }
  for(size_t i = 0; i < mVulkanFrameBuffers.Size(); ++i)
  {
    delete mVulkanFrameBuffers[i];
  }
  for(size_t i = 0; i < mRenderPasses.Size(); ++i)
  {
    delete mRenderPasses[i];
  }
}

void VulkanResourcePool::Clear()
{
  mVulkanFrameBuffers.Clear();
  mRenderPasses.Clear();
  mVulkanImageViews.Clear();
}

