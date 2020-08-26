#pragma once

#include "VulkanStandard.hpp"

struct VulkanRuntimeData;
class VulkanImage;
class VulkanImageView;
class VulkanFrameBuffer;
class VulkanRenderPass;

//-------------------------------------------------------------------VulkanResourcePool
class VulkanResourcePool
{
public:
  void Add(VulkanImage* image);
  void Add(VulkanImageView* imageView);
  void Add(VulkanRenderPass* renderPass);
  void Add(VulkanFrameBuffer* frameBuffer);

  void Free(VulkanRuntimeData& runtimeData);
  void Clear();

private:
  Array<VulkanImage*> mVulkanImages;
  Array<VulkanImageView*> mVulkanImageViews;
  Array<VulkanFrameBuffer*> mVulkanFrameBuffers;
  Array<VulkanRenderPass*> mRenderPasses;
};
