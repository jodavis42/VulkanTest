#pragma once

#include "GraphicsStandard.hpp"
#include "Zilch/Zilch.hpp"
#include "GraphicsBufferTypes.hpp"
#include "RenderGroup.hpp"
#include "TextureFormat.hpp"
#include "RenderTarget.hpp"
#include "RenderPipelineSettings.hpp"

struct Mesh;
struct ZilchMaterial;
struct ZilchShader;
struct Graphical;
class GraphicsSpace;
struct RenderTaskEvent;
struct ViewBlock;

enum class RenderTaskType : char
{
  ClearTarget,
  RenderGroup
};

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

  Vec4 mClearColor = Vec4(0, 0, 0, 1);
  float mDepth = 1.0f;
  uint32_t mStencil = 0;
  Zilch::HandleOf<RenderTarget> mTarget;
};

//-------------------------------------------------------------------RenderGroupRenderTask
struct RenderGroupRenderTask : public RenderTask
{
  ZilchDeclareType(RenderGroupRenderTask, Zilch::TypeCopyMode::ReferenceType);
  using RenderGroupHandle = Zilch::HandleOf<RenderGroup>;
  RenderGroupRenderTask();

  void Add(const Graphical* graphical);
  void AddRenderGroup(const RenderGroupHandle& renderGroup);

  Zilch::HandleOf<RenderTarget> mColorTarget0;
  Zilch::HandleOf<RenderTarget> mDepthTarget;
  RenderPipelineSettings mRenderPipelineSettings;
  Array<GraphicalFrameData> mFrameData;
  Array<RenderGroupHandle> mRenderGroups;
};

//-------------------------------------------------------------------RenderTaskEvent
struct RenderTaskEvent : public Zilch::EventData
{
  ZilchDeclareType(RenderTaskEvent, Zilch::TypeCopyMode::ReferenceType);
  ~RenderTaskEvent();

  Zilch::Integer2 GetViewportSize() const;
  Zilch::HandleOf<RenderTarget> GetFinalTarget(const Zilch::Integer2& textureSize, TextureFormat::Enum format);
  Zilch::HandleOf<RenderTarget> GetRenderTarget(const Zilch::Integer2& textureSize, TextureFormat::Enum format);

  ClearTargetRenderTask* CreateClearTargetRenderTask();
  RenderGroupRenderTask* CreateRenderGroupRenderTask();
  RenderGroupRenderTask* CreateRenderGroupRenderTask(const RenderPipelineSettings& renderPipelineSettings);

  ViewBlock* mViewBlock = nullptr;
  GraphicsSpace* mGraphicsSpace = nullptr;
  Array<Zilch::HandleOf<RenderTask>> mRenderTasks;
  uint32_t mLastRenderTargetId = 0;
};

namespace Events
{
ZilchDeclareEvent(CollectRenderTasks, RenderTaskEvent);
}//namespace Events
