#include "Precompiled.hpp"

#include "GraphicsSpace.hpp"
#include "GraphicsEngine.hpp"
#include "GraphicsBufferTypes.hpp"
#include "GraphicalEntry.hpp"
#include "RenderTasks.hpp"
#include "RenderQueue.hpp"
#include "Camera.hpp"

GraphicsSpace::GraphicsSpace()
{
  Camera* camera = new Camera();
  mCameras.PushBack(camera);
}

GraphicsSpace::~GraphicsSpace()
{
  for(Camera* camera : mCameras)
    delete camera;
  mCameras.Clear();
  for(Model* model : mModels)
    delete model;
  mModels.Clear();
}

void GraphicsSpace::Add(Model* model)
{
  mModels.PushBack(model);
  model->mSpace = this;
}

void GraphicsSpace::Update(UpdateEvent& e)
{
  mTotalTimeElapsed += e.mDt;
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
  
    RenderTaskEvent& renderTaskEvent = viewBlock.mRenderTaskEvent;
    renderTaskEvent.mGraphicsSpace = this;
  
    Array<GraphicalEntry> entries;
    entries.Reserve(mModels.Size());
    for(Model* model : mModels)
    {
      GraphicalEntry& entry = entries.PushBack();
      entry.mGraphical = model;
      entry.mSortId = 0;
    }
    Zero::Sort(mModels.All());
  
    renderTaskEvent.CreateClearTargetRenderTask();
    RenderGroupRenderTask* renderGroupTask = renderTaskEvent.CreateRenderGroupRenderTask();
    for(const GraphicalEntry& entry : entries)
      renderGroupTask->Add(entry.mGraphical);
  }
}
