#include "Precompiled.hpp"

#include "RenderTasks.hpp"
#include "Model.hpp"
#include "Mesh.hpp"
#include "ZilchMaterial.hpp"
#include "ZilchShader.hpp"

RenderTask::RenderTask(RenderTaskType taskType)
  : mTaskType(taskType)
{

}

ClearTargetRenderTask::ClearTargetRenderTask()
  : RenderTask(RenderTaskType::ClearTarget)
{

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

ClearTargetRenderTask* RenderTaskEvent::CreateClearTargetRenderTask()
{
  ClearTargetRenderTask* result = new ClearTargetRenderTask();
  result->mOwningEvent = this;
  mRenderTasks.PushBack(result);
  return result;
}

RenderGroupRenderTask* RenderTaskEvent::CreateRenderGroupRenderTask()
{
  RenderGroupRenderTask* result = new RenderGroupRenderTask();
  result->mOwningEvent = this;
  mRenderTasks.PushBack(result);
  return result;
}

RenderTaskEvent::~RenderTaskEvent()
{
  for(RenderTask* renderTask : mRenderTasks)
    delete renderTask;
  mRenderTasks.Clear();
}
