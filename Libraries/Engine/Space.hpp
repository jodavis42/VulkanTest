#pragma once

#include "EngineStandard.hpp"

#include "Composition.hpp"

class Engine;
class GameSession;

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
  GameSession* GetGame() const;
  Engine* GetEngine() const;

  using CompositionHandle = Zilch::HandleOf<Composition>;
  Array<CompositionHandle> mCompositions;
  Array<CompositionHandle> mCompositionsToDestroy;
  GameSession* mGame = nullptr;
};
