#pragma once

#include "EngineStandard.hpp"

#include "Composition.hpp"

//-------------------------------------------------------------------Engine
class Engine : public Composition
{
public:
  ZilchDeclareType(Engine, Zilch::TypeCopyMode::ReferenceType);

  void Add(Space* space);
  void QueueForDestruction(Space* space);
  void DestroyQueuedCompositions();

  void InitializeCompositions(const CompositionInitializer& initializer);

  using SpaceHandle = Zilch::HandleOf<Space>;
  Array<SpaceHandle> mSpaces;
  Array<SpaceHandle> mSpacesToDestroy;
};
