#include "Precompiled.hpp"

#include "RenderTasks.hpp"
#include "Model.hpp"
#include "Mesh.hpp"
#include "ZilchMaterial.hpp"
#include "ZilchShader.hpp"
#include "RenderQueue.hpp"

//-------------------------------------------------------------------RenderTask
ZilchDefineType(RenderTask, builder, type)
{
  ZilchBindDestructor();
}

RenderTask::RenderTask(RenderTaskType taskType)
  : mTaskType(taskType)
{

}

//-------------------------------------------------------------------ClearTargetRenderTask
ZilchDefineType(ClearTargetRenderTask, builder, type)
{
  ZilchBindDefaultCopyDestructor();

  ZilchBindField(mClearColor);
  ZilchBindField(mDepth);
  ZilchBindField(mStencil);
  ZilchBindField(mTarget);
}

ClearTargetRenderTask::ClearTargetRenderTask()
  : RenderTask(RenderTaskType::ClearTarget)
{

}

//-------------------------------------------------------------------RenderGroupRenderTask
ZilchDefineType(RenderGroupRenderTask, builder, type)
{
  ZilchBindDefaultCopyDestructor();

  ZilchBindMethod(AddRenderGroup);
  ZilchBindField(mColorTarget0);
  ZilchBindField(mDepthTarget);
}

RenderGroupRenderTask::RenderGroupRenderTask()
  : RenderTask(RenderTaskType::RenderGroup)
{

}

void RenderGroupRenderTask::Add(const Graphical* graphical)
{
  GraphicalFrameData& frameData = mFrameData.PushBack();
  graphical->FilloutFrameData(frameData);
}

void RenderGroupRenderTask::AddRenderGroup(const RenderGroupHandle& renderGroup)
{
  mRenderGroups.PushBack(renderGroup);
}

//-------------------------------------------------------------------RenderTaskEvent
ZilchDefineType(RenderTaskEvent, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  ZilchBindGetter(ViewportSize);
  ZilchBindMethod(GetFinalTarget);
  ZilchBindMethod(GetRenderTarget);

  ZilchBindMethod(CreateClearTargetRenderTask);
  ZilchBindOverloadedMethod(CreateRenderGroupRenderTask, ZilchInstanceOverload(RenderGroupRenderTask*));
  ZilchBindOverloadedMethod(CreateRenderGroupRenderTask, ZilchInstanceOverload(RenderGroupRenderTask*, const RenderPipelineSettings&));
}

RenderTaskEvent::~RenderTaskEvent()
{
  for(Zilch::HandleOf<RenderTask>& renderTask : mRenderTasks)
    renderTask.Delete();
  mRenderTasks.Clear();
}

Zilch::Integer2 RenderTaskEvent::GetViewportSize() const
{
  Zilch::Integer2 result;
  result.x = static_cast<int>(mViewBlock->mViewportSize.x);
  result.y = static_cast<int>(mViewBlock->mViewportSize.y);
  return result;
}

Zilch::HandleOf<RenderTarget> RenderTaskEvent::GetFinalTarget(const Zilch::Integer2& textureSize, TextureFormat::Enum format)
{
  Zilch::HandleOf<RenderTarget> result = ZilchAllocate(RenderTarget);
  result->mId = RenderTarget::mFinalTargetId;
  result->mSize = textureSize;
  result->mFormat = format;
  return result;
}

Zilch::HandleOf<RenderTarget> RenderTaskEvent::GetRenderTarget(const Zilch::Integer2& textureSize, TextureFormat::Enum format)
{
  Zilch::HandleOf<RenderTarget> result = ZilchAllocate(RenderTarget);
  result->mId = ++mLastRenderTargetId;
  result->mSize = textureSize;
  result->mFormat = format;
  return result;
}

ClearTargetRenderTask* RenderTaskEvent::CreateClearTargetRenderTask()
{
  Zilch::HandleOf<ClearTargetRenderTask> result = ZilchAllocate(ClearTargetRenderTask);
  result->mOwningEvent = this;
  mRenderTasks.PushBack(result);
  return result;
}

RenderGroupRenderTask* RenderTaskEvent::CreateRenderGroupRenderTask()
{
  Zilch::HandleOf<RenderGroupRenderTask> result = ZilchAllocate(RenderGroupRenderTask);
  result->mOwningEvent = this;
  mRenderTasks.PushBack(result);
  return result;
}

RenderGroupRenderTask* RenderTaskEvent::CreateRenderGroupRenderTask(const RenderPipelineSettings& renderPipelineSettings)
{
  Zilch::HandleOf<RenderGroupRenderTask> result = ZilchAllocate(RenderGroupRenderTask);
  result->mOwningEvent = this;
  result->mRenderPipelineSettings = renderPipelineSettings;
  mRenderTasks.PushBack(result);
  return result;
}

namespace Events
{
ZilchDefineEvent(CollectRenderTasks);
}//namespace Events
