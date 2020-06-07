#pragma once

#include "EngineStandard.hpp"

#include "Composition.hpp"

class Engine;

//-------------------------------------------------------------------Space
class Space : public Composition
{
public:
  ZilchDeclareType(Space, Zilch::TypeCopyMode::ReferenceType);

  Space();
  void Add(Composition* composition);
  void QueueForDestruction(Composition* composition);
  void DestroyQueuedCompositions();

  void InitializeCompositions(const CompositionInitializer& initializer);
  Engine* GetEngine() const;

  using CompositionHandle = Zilch::HandleOf<Composition>;
  Array<CompositionHandle> mCompositions;
  Array<CompositionHandle> mCompositionsToDestroy;
  Engine* mEngine = nullptr;
};
