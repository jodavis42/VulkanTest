#include "Precompiled.hpp"

#include "Engine.hpp"

#include "GameSession.hpp"
#include "UpdateEvent.hpp"

//-------------------------------------------------------------------Engine
ZilchDefineType(Engine, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  builder.AddSendsEvent(type, Events::EngineUpdate, ZilchTypeId(Zilch::EventData));
}

void Engine::Add(GameSession* game)
{
  game->mEngine = this;
  mGameSessions.PushBack(game);
}

void Engine::QueueForDestruction(GameSession* game)
{
  size_t index = mGameSessions.FindIndex(GameSessionHandle(game));
  Math::Swap(mGameSessions[index], mGameSessions[mGameSessions.Size() - 1]);
  mGameSessions.PopBack();
  mGameSessionsToDestroy.PushBack(game);
}

void Engine::DestroyQueuedGameSessions()
{
  for(GameSessionHandle game : mGameSessionsToDestroy)
    game.Delete();
  mGameSessionsToDestroy.Clear();
}

void Engine::InitializeCompositions(const CompositionInitializer& initializer)
{
  for(GameSession* game : mGameSessions)
  {
    game->Initialize(initializer);
  }
}

void Engine::Update(float dt)
{
  Zilch::HandleOf<Zilch::EventData> toSend = ZilchAllocate(Zilch::EventData);
  toSend->EventName = Events::EngineUpdate;
  Zilch::EventSend(this, toSend->EventName, toSend);

  for(GameSession* game : mGameSessions)
  {
    game->Update(dt);
  }
  DestroyQueuedGameSessions();
}
