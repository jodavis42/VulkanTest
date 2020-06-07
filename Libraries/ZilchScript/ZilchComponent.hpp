#pragma once

#include "EngineStandard.hpp"

#include "Component.hpp"

//-------------------------------------------------------------------ZilchComponent
struct ZilchComponent : public Component
{
public:
  ZilchDeclareInheritableType(ZilchComponent, Zilch::TypeCopyMode::ReferenceType);

  ~ZilchComponent();
  virtual void Initialize(const CompositionInitializer& initializer) override;

  void OnLogicUpdate(Zilch::EventData* event);
};
