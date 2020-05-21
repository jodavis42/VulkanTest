#pragma once

#include "GraphicsStandard.hpp"
#include "Zilch/Zilch.hpp"
#include "GraphicsBufferTypes.hpp"

struct Mesh;
struct ZilchMaterial;
struct ZilchShader;
struct Graphical;
class GraphicsSpace;
struct RenderTaskEvent;

enum class RenderTaskType : char
{
  ClearTarget,
  RenderGroup
};

using TargetId = uint32_t;
using ObjectId = size_t;
constexpr TargetId mInvalidTarget = static_cast<TargetId>(-1);

struct GraphicalFrameData
{
  Matrix4 mLocalToWorld = Matrix4::cIdentity;
  const Mesh* mMesh = nullptr;
  const ZilchShader* mZilchShader = nullptr;
  const ZilchMaterial* mZilchMaterial = nullptr;
};

struct GraphicalViewData
{
  Zilch::Real4x4 mWorldToView;
  Zilch::Real4x4 mViewToPerspective;
  Zilch::Real4x4 mPerspectiveToApiPerspective;
};

struct RenderSettings
{
  TargetId mColorTargetId = mInvalidTarget;
};

struct RenderTask
{
  RenderTask(RenderTaskType taskType);
  virtual ~RenderTask() {}
  RenderTaskType mTaskType;
  RenderTaskEvent* mOwningEvent = nullptr;
};

struct ClearTargetRenderTask : public RenderTask
{
  ClearTargetRenderTask();

  RenderSettings mRenderSettings;
  Vec4 mClearColor = Vec4(0, 0, 0, 1);
  float mDepthdepth = 1.0f;
  uint32_t mStencil = 0;
};

struct RenderGroupRenderTask : public RenderTask
{
  RenderGroupRenderTask();

  void Add(const Graphical* graphical);

  RenderSettings mRenderSettings;
  Array<GraphicalFrameData> mFrameData;
};

struct RenderTaskEvent
{
  ~RenderTaskEvent();

  ClearTargetRenderTask* CreateClearTargetRenderTask();
  RenderGroupRenderTask* CreateRenderGroupRenderTask();

  GraphicsSpace* mGraphicsSpace = nullptr;
  Array<RenderTask*> mRenderTasks;
};
