#pragma once

#include "GraphicsStandard.hpp"
#include "Engine/Component.hpp"
#include "Engine/UpdateEvent.hpp"

struct Camera;
struct Model;
struct GraphicsEngine;
struct RenderFrame;
struct RenderQueue;
struct RenderGroupRenderTask;
struct RenderTaskEvent;
struct GraphicalEntry;

class GraphicsSpace : public Component
{
public:
  ZilchDeclareType(GraphicsSpace, Zilch::TypeCopyMode::ReferenceType);

  GraphicsSpace();
  ~GraphicsSpace();

  virtual void Initialize(const CompositionInitializer& initializer) override;
  virtual void OnDestroy() override;

  void Add(Model* model);
  void Remove(Model* model);
  void Add(Camera* camera);
  void Remove(Camera* camera);

  void OnLogicUpdate(UpdateEvent* e);
  void RenderQueueUpdate(RenderQueue& renderQueue);
  void ProcessRenderGroupTasks(RenderTaskEvent& renderTaskEvent) const;
  void CollectRenderGroups(RenderGroupRenderTask* renderGroupTask, Array<GraphicalEntry>& entries) const;

  float mTotalTimeElapsed = 0.0;
  Array<Camera*> mCameras;
  Array<Model*> mModels;
  String mName;
  GraphicsEngine* mEngine = nullptr;
};
