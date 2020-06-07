#pragma once

#include "EngineStandard.hpp"
#include "Zilch/Zilch.hpp"

class CompositionInitializer;
class Composition;
class Space;
class Engine;

//-------------------------------------------------------------------Component
class Component : public Zilch::EventHandler
{
public:
  ZilchDeclareType(Component, Zilch::TypeCopyMode::ReferenceType);

  Component() {};
  virtual ~Component() {}

  virtual void Initialize(const CompositionInitializer& initializer) {}
  virtual void OnDestroy() {}

  Composition* GetOwner() const;
  Space* GetSpace() const;
  Engine* GetEngine() const;

  Composition* mOwner = nullptr;
};
