#pragma once

#include "EngineStandard.hpp"

#include "Composition.hpp"

class Engine;

//-------------------------------------------------------------------GameSession
class GameSession : public Composition
{
public:
  ZilchDeclareType(GameSession, Zilch::TypeCopyMode::ReferenceType);

  void Add(Space* space);
  void QueueForDestruction(Space* space);
  void DestroyQueuedSpaces();

  void InitializeCompositions(const CompositionInitializer& initializer);
  void Update(float dt);

  Engine* GetEngine() const;

  using SpaceHandle = Zilch::HandleOf<Space>;
  Array<SpaceHandle> mSpaces;
  Array<SpaceHandle> mSpacesToDestroy;
  Engine* mEngine = nullptr;
};
