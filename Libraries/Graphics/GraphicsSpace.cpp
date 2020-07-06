#include "Precompiled.hpp"

#include "GraphicsSpace.hpp"

#include "Engine/Composition.hpp"
#include "Engine/Engine.hpp"

#include "GraphicsEngine.hpp"
#include "GraphicsBufferTypes.hpp"
#include "GraphicalEntry.hpp"
#include "RenderTasks.hpp"
#include "RenderQueue.hpp"
#include "Camera.hpp"

ZilchDefineType(GraphicsSpace, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  builder.AddSendsEvent(type, Events::CollectRenderTasks, ZilchTypeId(RenderTaskEvent));
}

GraphicsSpace::GraphicsSpace()
{
}

GraphicsSpace::~GraphicsSpace()
{
  mCameras.Clear();
  mModels.Clear();
}

void GraphicsSpace::Initialize(const CompositionInitializer& initializer)
{
  Zilch::EventConnect(GetOwner(), Events::LogicUpdate, &GraphicsSpace::OnLogicUpdate, this);

  mEngine = GetEngine()->Has<GraphicsEngine>();
  mEngine->Add(this);
}

void GraphicsSpace::OnDestroy()
{
  mEngine->Remove(this);
}

void GraphicsSpace::Add(Model* model)
{
  mModels.PushBack(model);
  model->mSpace = this;
}

void GraphicsSpace::Remove(Model* model)
{
  size_t index = mModels.FindIndex(model);
  Math::Swap(mModels[index], mModels[mModels.Size() - 1]);
  mModels.PopBack();
}

void GraphicsSpace::Add(Camera* camera)
{
  mCameras.PushBack(camera);
}

void GraphicsSpace::Remove(Camera* camera)
{
  size_t index = mCameras.FindIndex(camera);
  Math::Swap(mCameras[index], mCameras[mCameras.Size() - 1]);
  mCameras.PopBack();
}

void GraphicsSpace::OnLogicUpdate(UpdateEvent* e)
{
  mTotalTimeElapsed += e->mDt;
}

void GraphicsSpace::RenderQueueUpdate(RenderQueue& renderQueue)
{
  Renderer* renderer = mEngine->GetRenderer();

  FrameBlock& frameBlock = renderQueue.mFrameBlocks.PushBack();
  frameBlock.mFrameTime = mTotalTimeElapsed;
  frameBlock.mLogicTime = mTotalTimeElapsed;

  for(const Camera* camera : mCameras)
  {
    ViewBlock& viewBlock = renderQueue.mViewBlocks.PushBack();
    viewBlock.mFrameBlockId = static_cast<uint32_t>(renderQueue.mFrameBlocks.Size()) - 1;
    camera->FilloutViewBlock(renderer, viewBlock);
  
    Zilch::HandleOf<RenderTaskEvent> renderTaskEvent = ZilchAllocate(RenderTaskEvent);
    viewBlock.mRenderTaskEvent = renderTaskEvent;
    renderTaskEvent->mGraphicsSpace = this;
    renderTaskEvent->EventName = Events::CollectRenderTasks;
    renderTaskEvent->mViewBlock = &viewBlock;

    Zilch::EventSend(GetOwner(), renderTaskEvent->EventName, renderTaskEvent);
  
    ProcessRenderGroupTasks(renderTaskEvent);
  }
}

void GraphicsSpace::ProcessRenderGroupTasks(RenderTaskEvent& renderTaskEvent) const
{
  for(RenderTask* renderTask : renderTaskEvent.mRenderTasks)
  {
    if(renderTask->mTaskType != RenderTaskType::RenderGroup)
      continue;
    
    RenderGroupRenderTask* renderGroupTask = (RenderGroupRenderTask*)renderTask;
    Array<GraphicalEntry> entries;
    CollectRenderGroups(renderGroupTask, entries);
    
    for(const GraphicalEntry& entry : entries)
    {
      renderGroupTask->Add(entry.mGraphical);
    }
  }
}

void GraphicsSpace::CollectRenderGroups(RenderGroupRenderTask* renderGroupTask, Array<GraphicalEntry>& entries) const
{
  entries.Reserve(mModels.Size());
  for(Model* model : mModels)
  {
    for(auto renderGroup : renderGroupTask->mRenderGroups)
    {
      if(model->mRenderGroupSet.Contains(renderGroup))
      {
        GraphicalEntry& entry = entries.PushBack();
        entry.mGraphical = model;
        entry.mSortId = 0;
        break;
      }
    }
  }
  Zero::Sort(entries.All());
}
