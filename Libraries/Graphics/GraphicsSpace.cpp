#include "Precompiled.hpp"

#include "GraphicsSpace.hpp"
#include "GraphicsEngine.hpp"
#include "GraphicsBufferTypes.hpp"
#include "GraphicalEntry.hpp"
#include "RenderTasks.hpp"
#include "RenderQueue.hpp"

static Matrix4 GenerateLookAt(const Vec3& eye, const Vec3& center, const Vec3& worldUp)
{
  Vec3 forward = Vec3::Normalized(center - eye);
  Vec3 right = Vec3::Normalized(Vec3::Cross(forward, worldUp));
  Vec3 actualUp = Vec3::Cross(right, forward);

  Matrix4 result;
  result.SetIdentity();
  result.m00 = right.x;
  result.m10 = right.y;
  result.m20 = right.z;
  result.m01 = actualUp.x;
  result.m11 = actualUp.y;
  result.m21 = actualUp.z;
  result.m02 = -forward.x;
  result.m12 = -forward.y;
  result.m22 = -forward.z;
  result.m30 = -Vec3::Dot(right, eye);
  result.m31 = -Vec3::Dot(actualUp, eye);
  result.m32 = Vec3::Dot(forward, eye);
  return result;
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

  ViewBlock& viewBlock = renderQueue.mViewBlocks.PushBack();
  viewBlock.mFrameBlockId = static_cast<uint32_t>(renderQueue.mFrameBlocks.Size()) - 1;

  RenderTaskEvent& renderTaskEvent = viewBlock.mRenderTaskEvent;
  renderTaskEvent.mGraphicsSpace = this;

  float nearDistance = 0.1f;
  float farDistance = 10.0f;
  size_t width, height;
  float aspectRatio;
  renderer->GetShape(width, height, aspectRatio);
  float fov = Math::DegToRad(45.0f);

  viewBlock.mPerspectiveToApiPerspective.SetIdentity();
  viewBlock.mWorldToView = GenerateLookAt(Vec3(5.0f, 5.0f, 5.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
  viewBlock.mViewToPerspective = renderer->BuildPerspectiveMatrix(fov, aspectRatio, nearDistance, farDistance);
  viewBlock.mWorldToView.Transpose();
  viewBlock.mViewToPerspective.Transpose();

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
