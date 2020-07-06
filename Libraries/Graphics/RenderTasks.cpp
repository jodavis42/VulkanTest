#include "Precompiled.hpp"

#include "RenderTasks.hpp"
#include "Model.hpp"
#include "Mesh.hpp"
#include "ZilchMaterial.hpp"
#include "ZilchShader.hpp"

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
  ZilchBindMethod(CreateClearTargetRenderTask);
  ZilchBindMethod(CreateRenderGroupRenderTask);
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

RenderTaskEvent::~RenderTaskEvent()
{
  for(Zilch::HandleOf<RenderTask>& renderTask : mRenderTasks)
    renderTask.Delete();
  mRenderTasks.Clear();
}

namespace Events
{
ZilchDefineEvent(CollectRenderTasks);
}//namespace Events
