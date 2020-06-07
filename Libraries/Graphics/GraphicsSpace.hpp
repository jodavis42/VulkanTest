#pragma once

#include "GraphicsStandard.hpp"
#include "Engine/Component.hpp"
#include "Engine/UpdateEvent.hpp"

struct Camera;
struct Model;
struct GraphicsEngine;
struct RenderFrame;
struct RenderQueue;

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
  void OnLogicUpdate(UpdateEvent* e);
  void RenderQueueUpdate(RenderQueue& renderQueue);

  float mTotalTimeElapsed = 0.0;
  Array<Camera*> mCameras;
  Array<Model*> mModels;
  String mName;
  GraphicsEngine* mEngine = nullptr;
};
