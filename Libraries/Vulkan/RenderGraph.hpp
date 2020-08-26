#pragma once

#include "VulkanStandard.hpp"

#include "RenderPipelineSettings.hpp"
#include "RenderTasks.hpp"

class VulkanRenderPass;
class VulkanRenderer;
class VulkanImage;
class VulkanImageView;
class VulkanFrameBuffer;
class VulkanSampler;
class VulkanRenderPass;
class VulkanRenderPassInfo;
class VulkanPipeline;
struct RenderQueue;
struct RenderTaskEvent;
struct RenderGroupRenderTask;
struct ViewBlock;

//-------------------------------------------------------------------RenderGraphCreationInfo
struct RenderGraphCreationInfo
{
  VkDevice mDevice = VK_NULL_HANDLE;
};

//-------------------------------------------------------------------RenderRanges
class RenderRanges
{
public:
  void Build(VulkanRenderer& renderer, const RenderPipelineSettings& pipelineSettings, VulkanRenderPass& renderPass, const Array<GraphicalFrameData>& frameDataList, size_t startIndex = 0);
  void Build(VulkanRenderer& renderer, const RenderPipelineSettings& pipelineSettings, VulkanRenderPass& renderPass, const Array<GraphicalFrameData>& frameDataList, size_t startIndex, size_t endIndex);

  struct RenderRange
  {
    VulkanRenderPass* mRenderPass = nullptr;
    const VulkanPipeline* mPipeline = nullptr;
    size_t mStart = 0;
    size_t mEnd = 0;
  };

  Array<RenderRange> mRenderRanges;
};

//-------------------------------------------------------------------RenderGraph
class RenderGraph
{
public:
  RenderGraph(RenderGraphCreationInfo& creationInfo);
  ~RenderGraph();

  void Free();

  void Bake(VulkanRenderer& renderer, const RenderQueue& renderQueue);

  struct ClearColor
  {
    Vec4 mColor = Vec4::cZero;
  };
  struct ClearDepth
  {
    float mDepth = 0;
    uint32_t mStencil = 0;
  };
  struct Barrier
  {
    VulkanImage* mImage = nullptr;
    VkImageLayout mOldLayout;
    VkImageLayout mNewLayout;
    VkAccessFlags mSrcAccessMask;
    VkAccessFlags mDstAccessMask;
    VkPipelineStageFlags mSrcStageFlags;
    VkPipelineStageFlags mDstStageFlags;
  };
  struct PhysicalPass
  {
    const ViewBlock* mViewBlock = nullptr;
    VulkanFrameBuffer* mFrameBuffer = nullptr;
    Array<GraphicalFrameData> mFrameData;
    RenderRanges mRenderRanges;
    VulkanRenderPass* mRenderPass = nullptr;

    Array<ClearColor> mClearColors;
    Array<ClearDepth> mClearDepths;
  };
  Array<PhysicalPass> mPasses;

private:
  static constexpr size_t mFinalImageColorViewId = -2;
  static constexpr size_t mFinalImageDepthViewId = -3;

  struct LogicalImageResource
  {
    const RenderTarget* mRenderTarget = nullptr;
    VulkanImageView* mPhysicalImage = nullptr;
    VkAttachmentLoadOp mLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkImageLayout mInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout mFinalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    bool mUseClearColor = false;
    ClearColor mClearColor;
    ClearDepth mClearDepth;
  };
  struct LogicalPassRange
  {
    size_t mStart = 0;
    size_t mEnd = 0;
    const RenderTask* mRenderTask = nullptr;
  };
  struct LogicalPass
  {
    const ViewBlock* mViewBlock = nullptr;
    RenderTaskType mTaskType;
    Array<LogicalImageResource> mColorImages;
    Array<LogicalImageResource> mDepthImages;
    Array<GraphicalFrameData> mFrameData;
    Array<LogicalPassRange> mLogicalRanges;
  };
  Array<LogicalPass> mLogicalPasses;

  void CreateLogicalPasses(VulkanRenderer& renderer, const ViewBlock* viewBlock, const RenderTaskEvent& renderTaskEvent);
  void AssignLogicalIds(VulkanRenderer& renderer);
  void MergeLogicalPasses(VulkanRenderer& renderer);
  void ComputeLogicalLayouts(VulkanRenderer& renderer);
  void AssignPhysicalResources(VulkanRenderer& renderer);
  void CreatePhysicalPasses(VulkanRenderer& renderer);

  void CreateRenderPassInfo(const LogicalPass& logicalPass, VulkanRenderPassInfo& renderPassInfo);

  VkDevice mDevice = VK_NULL_HANDLE;
};