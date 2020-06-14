#pragma once

#include "EngineStandard.hpp"

#include "Composition.hpp"

class GameSession;
class IApplication;

//-------------------------------------------------------------------Engine
class Engine : public Composition
{
public:
  ZilchDeclareType(Engine, Zilch::TypeCopyMode::ReferenceType);

  void Add(GameSession* game);
  void QueueForDestruction(GameSession* game);
  void DestroyQueuedGameSessions();

  void InitializeCompositions(const CompositionInitializer& initializer);
  void Update(float dt);

  using GameSessionHandle = Zilch::HandleOf<GameSession>;
  Array<GameSessionHandle> mGameSessions;
  Array<GameSessionHandle> mGameSessionsToDestroy;
  IApplication* mApplication = nullptr;
};
