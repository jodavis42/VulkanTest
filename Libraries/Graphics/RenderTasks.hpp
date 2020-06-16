#pragma once

#include "GraphicsStandard.hpp"
#include "Zilch/Zilch.hpp"
#include "GraphicsBufferTypes.hpp"
#include "RenderGroup.hpp"

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

//-------------------------------------------------------------------RenderTask
struct RenderTask
{
  ZilchDeclareType(RenderTask, Zilch::TypeCopyMode::ReferenceType);
  RenderTask(RenderTaskType taskType);
  virtual ~RenderTask() {}
  RenderTaskType mTaskType;
  RenderTaskEvent* mOwningEvent = nullptr;
};

//-------------------------------------------------------------------ClearTargetRenderTask
struct ClearTargetRenderTask : public RenderTask
{
  ZilchDeclareType(ClearTargetRenderTask, Zilch::TypeCopyMode::ReferenceType);
  ClearTargetRenderTask();

  RenderSettings mRenderSettings;
  Vec4 mClearColor = Vec4(0, 0, 0, 1);
  float mDepthdepth = 1.0f;
  uint32_t mStencil = 0;
};

//-------------------------------------------------------------------RenderGroupRenderTask
struct RenderGroupRenderTask : public RenderTask
{
  ZilchDeclareType(RenderGroupRenderTask, Zilch::TypeCopyMode::ReferenceType);
  using RenderGroupHandle = Zilch::HandleOf<RenderGroup>;
  RenderGroupRenderTask();

  void Add(const Graphical* graphical);
  void AddRenderGroup(const RenderGroupHandle& renderGroup);

  RenderSettings mRenderSettings;
  Array<GraphicalFrameData> mFrameData;
  Array<RenderGroupHandle> mRenderGroups;
};

//-------------------------------------------------------------------RenderTaskEvent
struct RenderTaskEvent : public Zilch::EventData
{
  ZilchDeclareType(RenderTaskEvent, Zilch::TypeCopyMode::ReferenceType);
  ~RenderTaskEvent();

  ClearTargetRenderTask* CreateClearTargetRenderTask();
  RenderGroupRenderTask* CreateRenderGroupRenderTask();

  GraphicsSpace* mGraphicsSpace = nullptr;
  Array<Zilch::HandleOf<RenderTask>> mRenderTasks;
};

namespace Events
{
ZilchDeclareEvent(CollectRenderTasks, RenderTaskEvent);
}//namespace Events
