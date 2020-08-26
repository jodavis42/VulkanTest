#include "Precompiled.hpp"

#include "RenderGraph.hpp"

#include "Graphics/RenderQueue.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanImage.hpp"
#include "VulkanImageView.hpp"
#include "VulkanFrameBuffer.hpp"
#include "VulkanInitialization.hpp"

//-------------------------------------------------------------------RenderRanges
void RenderRanges::Build(VulkanRenderer& renderer, const RenderPipelineSettings& pipelineSettings, VulkanRenderPass& renderPass, const Array<GraphicalFrameData>& frameDataList, size_t startIndex)
{
  Build(renderer, pipelineSettings, renderPass, frameDataList, startIndex, frameDataList.Size());
}

void RenderRanges::Build(VulkanRenderer& renderer, const RenderPipelineSettings& pipelineSettings, VulkanRenderPass& renderPass, const Array<GraphicalFrameData>& frameDataList, size_t startIndex, size_t endIndex)
{
  VulkanMaterialPipelineCache& pipelineCache = *renderer.mInternal->mMaterialPipelineCache;
  VulkanShaderMaterial* prevShaderMaterial = nullptr;
  for(size_t i = startIndex; i < endIndex; ++i)
  {
    const GraphicalFrameData& frameData = frameDataList[i];
    VulkanShaderMaterial* vulkanShaderMaterial = renderer.mUniqueZilchShaderMaterialMap.FindValue(frameData.mZilchShader, nullptr);
    if(vulkanShaderMaterial == prevShaderMaterial)
      continue;

    if(!mRenderRanges.Empty())
      mRenderRanges.Back().mEnd = i;

    RenderRange& renderRange = mRenderRanges.PushBack();
    renderRange.mStart = i;
    renderRange.mEnd = i;
    renderRange.mRenderPass = &renderPass;

    VulkanPipelineCacheInfo pipelineCreationInfo;
    pipelineCreationInfo.mPipelineSettings = &pipelineSettings;
    pipelineCreationInfo.mRenderPassCookie = renderPass.GetCookie();
    pipelineCreationInfo.mVulkanShader = renderer.mZilchShaderMap[frameData.mZilchShader];
    pipelineCreationInfo.mVulkanShaderMaterial = vulkanShaderMaterial;

    VulkanPipeline* pipeline = pipelineCache.FindOrCreate(pipelineCreationInfo);
    renderRange.mPipeline = pipeline;
  }

  if(!mRenderRanges.Empty())
  {
    mRenderRanges.Back().mEnd = frameDataList.Size();
  }
}

//-------------------------------------------------------------------RenderGraph
RenderGraph::RenderGraph(RenderGraphCreationInfo& creationInfo)
{
  mDevice = creationInfo.mDevice;
}

RenderGraph::~RenderGraph()
{
  Free();
}

void RenderGraph::Free()
{

}

void RenderGraph::Bake(VulkanRenderer& renderer, const RenderQueue& renderQueue)
{
  for(const ViewBlock& viewBlock : renderQueue.mViewBlocks)
  {
    const FrameBlock& frameBlock = renderQueue.mFrameBlocks[viewBlock.mFrameBlockId];
    const RenderTaskEvent* renderTaskEvent = *viewBlock.mRenderTaskEvent;

    CreateLogicalPasses(renderer, &viewBlock, *renderTaskEvent);
    AssignLogicalIds(renderer);
    MergeLogicalPasses(renderer);
    ComputeLogicalLayouts(renderer);
    AssignPhysicalResources(renderer);
    CreatePhysicalPasses(renderer);
  }
}

void RenderGraph::CreateLogicalPasses(VulkanRenderer& renderer, const ViewBlock* viewBlock, const RenderTaskEvent& renderTaskEvent)
{
  if(renderTaskEvent.mRenderTasks.Empty())
    return;

  mLogicalPasses.Resize(renderTaskEvent.mRenderTasks.Size());

  size_t i = 0;
  for(const RenderTask* task : renderTaskEvent.mRenderTasks)
  {
    LogicalPass& logicalPass = mLogicalPasses[i];
    logicalPass.mTaskType = task->mTaskType;
    
    if(task->mTaskType == RenderTaskType::ClearTarget)
    {
      const ClearTargetRenderTask* clearTargetTask = reinterpret_cast<const ClearTargetRenderTask*>(task);
      logicalPass.mViewBlock = viewBlock;
      logicalPass.mLogicalRanges.PushBack({0, 0, task});
      LogicalImageResource colorImageResource;
      colorImageResource.mRenderTarget = clearTargetTask->mTarget;
      colorImageResource.mClearColor = {clearTargetTask->mClearColor};
      colorImageResource.mClearDepth.mDepth = clearTargetTask->mDepth;
      colorImageResource.mClearDepth.mStencil = clearTargetTask->mStencil;
      colorImageResource.mUseClearColor = true;
      if(clearTargetTask->mTarget->IsDepthFormat() || clearTargetTask->mTarget->IsDepthStencilFormat())
        logicalPass.mDepthImages.PushBack(colorImageResource);
      else
        logicalPass.mColorImages.PushBack(colorImageResource);
    }
    else if(task->mTaskType == RenderTaskType::RenderGroup)
    {
      const RenderGroupRenderTask* renderGroupTask = reinterpret_cast<const RenderGroupRenderTask*>(task);
      logicalPass.mViewBlock = viewBlock;
      logicalPass.mFrameData = renderGroupTask->mFrameData;
      logicalPass.mLogicalRanges.PushBack({0, logicalPass.mFrameData.Size(), task});

      LogicalImageResource colorImageResource;
      colorImageResource.mRenderTarget = renderGroupTask->mColorTarget0;
      logicalPass.mColorImages.PushBack(colorImageResource);
      LogicalImageResource depthImageResource;
      depthImageResource.mRenderTarget = *renderGroupTask->mDepthTarget;
      logicalPass.mDepthImages.PushBack(depthImageResource);
    }
    ++i;
  }
}

void RenderGraph::AssignLogicalIds(VulkanRenderer& renderer)
{
  for(LogicalPass& logicalPass : mLogicalPasses)
  {
    if(logicalPass.mTaskType == RenderTaskType::RenderGroup)
    {
      /*LogicalImageResource colorImageResource;
      colorImageResource.mId = mFinalImageColorViewId;
      logicalPass.mColorImages.PushBack(colorImageResource);
      LogicalImageResource depthImageResource;
      depthImageResource.mId = mFinalImageDepthViewId;
      logicalPass.mDepthImages.PushBack(depthImageResource);*/
    }
  }
}


void RenderGraph::MergeLogicalPasses(VulkanRenderer& renderer)
{
  Array<LogicalPass> passes;

  auto containsImageResourceRenderTargetFn = [&](const Array<LogicalImageResource>& resources, const LogicalImageResource& imageResource)
  {
    for(const LogicalImageResource& destResource : resources)
    {
      if(destResource.mRenderTarget->mId == imageResource.mRenderTarget->mId)
        return true;
    }
    return false;
  };

  auto containsAllImageResources = [&](const Array<LogicalImageResource>& dest, const Array<LogicalImageResource>& source)
  {
    if(dest.Size() != source.Size())
      return false;

    for(const LogicalImageResource& sourceResource : source)
    {
      if(!containsImageResourceRenderTargetFn(dest, sourceResource))
        return false;
    }
  };

  auto isPassMergeable = [&](LogicalPass& previousPass, LogicalPass& logicalPass) -> bool
  {
    if(logicalPass.mTaskType == RenderTaskType::ClearTarget && previousPass.mTaskType == RenderTaskType::ClearTarget)
      return true;

    if(!containsAllImageResources(previousPass.mColorImages, logicalPass.mColorImages))
      return false;

    if(!containsAllImageResources(previousPass.mDepthImages, logicalPass.mDepthImages))
      return false;

    return true;
  };
  auto findPassFn = [&](Array<LogicalPass>& previousPasses, LogicalPass& logicalPass)->LogicalPass*
  {
    for(LogicalPass& previousPass : previousPasses)
    {
      if(isPassMergeable(previousPass, logicalPass))
        return &previousPass;
    }
    return nullptr;
  };

  auto mergeImageFn = [](Array<LogicalImageResource>& dstImages, const Array<LogicalImageResource>& sourceImages)
  {
    for(const LogicalImageResource& sourceImage : sourceImages)
    {
      size_t foundIndex = -1;
      for(size_t i = 0; i < dstImages.Size(); ++i)
      {
        if(dstImages[i].mRenderTarget->mId == sourceImage.mRenderTarget->mId)
        {
          foundIndex = i;
          break;
        }
      }
      if(foundIndex == -1)
        dstImages.PushBack(sourceImage);
    }
  };

  for(LogicalPass& logicalPass : mLogicalPasses)
  {
    LogicalPass* passToMergeInto = findPassFn(passes, logicalPass);
    if(passToMergeInto == nullptr)
    {
      passes.PushBack(logicalPass);
      continue;
    }
    
    size_t lastIndex = passToMergeInto->mFrameData.Size();
    passToMergeInto->mFrameData.Insert(passToMergeInto->mFrameData.End(), logicalPass.mFrameData.All());
    for(size_t i = 0; i < logicalPass.mLogicalRanges.Size(); ++i)
    {
      auto&& range = logicalPass.mLogicalRanges[i];
      size_t startIndex = lastIndex + range.mStart;
      size_t endIndex = lastIndex + range.mEnd;
      passToMergeInto->mLogicalRanges.PushBack({startIndex, endIndex, range.mRenderTask});
      passToMergeInto->mTaskType = logicalPass.mTaskType;
    }
    mergeImageFn(passToMergeInto->mColorImages, logicalPass.mColorImages);
    mergeImageFn(passToMergeInto->mDepthImages, logicalPass.mDepthImages);
  }
  mLogicalPasses = passes;
}

void RenderGraph::ComputeLogicalLayouts(VulkanRenderer& renderer)
{
  HashMap<size_t, LogicalImageResource*> lastLayouts;
  auto computeTransitionLayout = [&lastLayouts](LogicalImageResource& imageResource, VkImageLayout transitionLayout, VkImageLayout finalLayout)
  {
    LogicalImageResource* lastImageInfo = lastLayouts.FindValue(imageResource.mRenderTarget->mId, nullptr);
    if(lastImageInfo != nullptr)
    {
      lastImageInfo->mFinalLayout = transitionLayout;
      imageResource.mLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      imageResource.mInitialLayout = lastImageInfo->mFinalLayout;
      imageResource.mFinalLayout = finalLayout;
    }
    else
    {
      imageResource.mLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      imageResource.mInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      imageResource.mFinalLayout = finalLayout;
    }
    lastLayouts[imageResource.mRenderTarget->mId] = &imageResource;
  };

  for(LogicalPass& logicalPass : mLogicalPasses)
  {
    if(logicalPass.mTaskType != RenderTaskType::RenderGroup)
      continue;

    for(LogicalImageResource& imageResource : logicalPass.mColorImages)
    {
      computeTransitionLayout(imageResource, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }
    for(LogicalImageResource& imageResource : logicalPass.mDepthImages)
    {
      computeTransitionLayout(imageResource, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }
  }
}

void RenderGraph::AssignPhysicalResources(VulkanRenderer& renderer)
{
  VulkanRuntimeData& runtimeData = *renderer.mInternal;
  uint32_t frameId = runtimeData.mCurrentImageIndex;
  VulkanRenderFrame& vulkanRenderFrame = runtimeData.mRenderFrames[frameId];
  VulkanImage* finalColorImage = runtimeData.mSwapChain->GetImage(frameId);

  VulkanImageViewInfo colorImageViewInfo;
  colorImageViewInfo.mFormat = runtimeData.mSwapChain->GetImageFormat();
  colorImageViewInfo.mAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
  VulkanImageView* colorImageView = new VulkanImageView(runtimeData.mDevice, finalColorImage, colorImageViewInfo);
  vulkanRenderFrame.mResources.Add(colorImageView);

  HashMap<RenderTarget::TargetId, VulkanImageView*> logicalImagesToPhysicalImages;
  logicalImagesToPhysicalImages[RenderTarget::mFinalTargetId] = colorImageView;

  auto logicalImageToPhysicalImageFn = [&](VulkanRenderer& renderer, LogicalImageResource& logicalImage)
  {
    const RenderTarget* renderTarget = logicalImage.mRenderTarget;
    logicalImage.mPhysicalImage = logicalImagesToPhysicalImages.FindValue(renderTarget->mId, nullptr);
    if(logicalImage.mPhysicalImage == nullptr)
    {
      logicalImage.mPhysicalImage = renderer.CreateRenderTargetImageView(*renderTarget);
      logicalImagesToPhysicalImages[renderTarget->mId] = logicalImage.mPhysicalImage;
    }

    ErrorIf(logicalImage.mPhysicalImage == nullptr, "Failed to create physical image");
  };

  for(LogicalPass& logicalPass : mLogicalPasses)
  {
    for(LogicalImageResource& logicalImage : logicalPass.mColorImages)
    {
      logicalImageToPhysicalImageFn(renderer, logicalImage);
    }
    for(LogicalImageResource& logicalImage : logicalPass.mDepthImages)
    {
      logicalImageToPhysicalImageFn(renderer, logicalImage);
    }
  }
}

void RenderGraph::CreatePhysicalPasses(VulkanRenderer& renderer)
{
  VulkanRuntimeData& runtimeData = *renderer.mInternal;
  uint32_t frameId = runtimeData.mCurrentImageIndex;
  VulkanRenderFrame& vulkanRenderFrame = runtimeData.mRenderFrames[frameId];

  Array<ClearColor> lastClearColors;
  Array<ClearDepth> lastClearDepths;

  for(const LogicalPass& logicalPass : mLogicalPasses)
  {
    if(logicalPass.mTaskType == RenderTaskType::ClearTarget)
    {
      const ClearTargetRenderTask* clearTargetTask = reinterpret_cast<const ClearTargetRenderTask*>(logicalPass.mLogicalRanges[0].mRenderTask);

      ClearColor clearColor;
      clearColor.mColor = clearTargetTask->mClearColor;
      lastClearColors.PushBack(clearColor);
      ClearDepth clearDepth;
      clearDepth.mDepth = clearTargetTask->mDepth;
      clearDepth.mStencil = clearTargetTask->mStencil;
      lastClearDepths.PushBack(clearDepth);
    }
    else if(logicalPass.mTaskType == RenderTaskType::RenderGroup)
    {
      VulkanRenderPassInfo renderPassInfo;
      CreateRenderPassInfo(logicalPass, renderPassInfo);

      VkExtent2D extent = runtimeData.mSwapChain->GetExtent();
      VulkanRenderPass* renderPass = renderer.mInternal->mRenderPassCache->FindOrCreate(renderPassInfo);
      VulkanFrameBuffer* frameBuffer = new VulkanFrameBuffer(runtimeData.mDevice, *renderPass, renderPassInfo, extent.width, extent.height);
      vulkanRenderFrame.mResources.Add(frameBuffer);

      PhysicalPass* physicalPass = &mPasses.PushBack();
      physicalPass->mViewBlock = logicalPass.mViewBlock;
      physicalPass->mRenderPass = renderPass;
      physicalPass->mFrameBuffer = frameBuffer;
      for(size_t i = 0; i < logicalPass.mColorImages.Size(); ++i)
      {
        if(logicalPass.mColorImages[i].mUseClearColor)
          physicalPass->mClearColors.PushBack(logicalPass.mColorImages[i].mClearColor);
      }
      for(size_t i = 0; i < logicalPass.mDepthImages.Size(); ++i)
      {
        if(logicalPass.mDepthImages[i].mUseClearColor)
          physicalPass->mClearDepths.PushBack(logicalPass.mDepthImages[i].mClearDepth);
      }
      physicalPass->mFrameData = logicalPass.mFrameData;
      for(auto&& range : logicalPass.mLogicalRanges)
      {
        const RenderGroupRenderTask* subRenderGroupTask = reinterpret_cast<const RenderGroupRenderTask*>(range.mRenderTask);
        physicalPass->mRenderRanges.Build(renderer, subRenderGroupTask->mRenderPipelineSettings, *renderPass, physicalPass->mFrameData, range.mStart, range.mEnd);
      }

      lastClearColors.Clear();
      lastClearDepths.Clear();
    }
  }
}

void RenderGraph::CreateRenderPassInfo(const LogicalPass& logicalPass, VulkanRenderPassInfo& renderPassInfo)
{
  // Setup Depth Target
  renderPassInfo.mDepthAttachment = logicalPass.mDepthImages[0].mPhysicalImage;
  renderPassInfo.mDepthDescription.mLoadOp = logicalPass.mDepthImages[0].mLoadOp;
  renderPassInfo.mDepthDescription.mInitialLayout = logicalPass.mDepthImages[0].mInitialLayout;
  renderPassInfo.mDepthDescription.mFinalLayout = logicalPass.mDepthImages[0].mFinalLayout;

  // Setup Color Targets
  renderPassInfo.mColorAttachmentCount = static_cast<byte>(logicalPass.mColorImages.Size());
  for(size_t i = 0; i < logicalPass.mColorImages.Size(); ++i)
  {
    renderPassInfo.mColorAttachments[i] = logicalPass.mColorImages[i].mPhysicalImage;
    renderPassInfo.mColorDescriptions[i].mLoadOp = logicalPass.mColorImages[i].mLoadOp;
    renderPassInfo.mColorDescriptions[i].mInitialLayout = logicalPass.mColorImages[i].mInitialLayout;
    renderPassInfo.mColorDescriptions[i].mFinalLayout = logicalPass.mColorImages[i].mFinalLayout;
  }

  auto& subPass = renderPassInfo.mSubPasses.PushBack();
  subPass.mColorAttachmentCount = 1;
  subPass.mColorAttachments[0] = 0;
}
