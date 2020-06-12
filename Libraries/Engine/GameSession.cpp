#include "Precompiled.hpp"

#include "GameSession.hpp"

#include "Space.hpp"
#include "Engine.hpp"
#include "TimeSpace.hpp"
#include "UpdateEvent.hpp"

//-------------------------------------------------------------------GameSession
ZilchDefineType(GameSession, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindGetter(Engine);
}

void GameSession::Add(Space* space)
{
  space->mGame = this;
  mSpaces.PushBack(space);
}

void GameSession::QueueForDestruction(Space* space)
{
  size_t index = mSpaces.FindIndex(SpaceHandle(space));
  Math::Swap(mSpaces[index], mSpaces[mSpaces.Size() - 1]);
  mSpaces.PopBack();
  mSpacesToDestroy.PushBack(space);
}

void GameSession::DestroyQueuedSpaces()
{
  for(SpaceHandle space : mSpacesToDestroy)
    space.Delete();
  mSpacesToDestroy.Clear();
}

void GameSession::InitializeCompositions(const CompositionInitializer& initializer)
{
  for(Space* space : mSpaces)
  {
    space->Initialize(initializer);
  }
}

void GameSession::Update(float dt)
{
  for(Space* space : mSpaces)
  {
    TimeSpace* timeSpace = space->Has<TimeSpace>();
    if(timeSpace != nullptr)
      timeSpace->Update(dt);
  }

  Zilch::HandleOf<Zilch::EventData> toSend = ZilchAllocate(Zilch::EventData);
  toSend->EventName = Events::EngineUpdate;
  Zilch::EventSend(this, toSend->EventName, toSend);

  for(Space* space : mSpaces)
    space->DestroyQueuedCompositions();
  DestroyQueuedSpaces();
}

Engine* GameSession::GetEngine() const
{
  return mEngine;
}
