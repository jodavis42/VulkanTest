#pragma once

#include "VulkanStandard.hpp"

struct VulkanRuntimeData;
class VulkanImageView;
class VulkanFrameBuffer;
class VulkanRenderPass;

//-------------------------------------------------------------------VulkanResourcePool
class VulkanResourcePool
{
public:
  void Add(VulkanImageView* imageView);
  void Add(VulkanRenderPass* renderPass);
  void Add(VulkanFrameBuffer* frameBuffer);

  void Free(VulkanRuntimeData& runtimeData);
  void Clear();

private:
  Array<VulkanImageView*> mVulkanImageViews;
  Array<VulkanFrameBuffer*> mVulkanFrameBuffers;
  Array<VulkanRenderPass*> mRenderPasses;
};
