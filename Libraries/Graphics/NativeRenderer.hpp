#pragma once

#include "GraphicsStandard.hpp"

#include "Engine/Component.hpp"

struct RenderTaskEvent;

//-------------------------------------------------------------------NativeRenderer
struct NativeRenderer : public Component
{
  ZilchDeclareType(NativeRenderer, Zilch::TypeCopyMode::ReferenceType);

  virtual void Initialize(const CompositionInitializer& initializer) override;
  void OnCollectRenderTasks(RenderTaskEvent* renderTaskEvent);

  bool mActive = true;
};
