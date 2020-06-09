#include "Precompiled.hpp"

#include "Engine.hpp"

#include "Space.hpp"
#include "TimeSpace.hpp"
#include "UpdateEvent.hpp"

//-------------------------------------------------------------------Engine
ZilchDefineType(Engine, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  builder.AddSendsEvent(type, Events::EngineUpdate, ZilchTypeId(Zilch::EventData));
}

void Engine::Add(Space* space)
{
  space->mEngine = this;
  mSpaces.PushBack(space);
}

void Engine::QueueForDestruction(Space* space)
{
  size_t index = mSpaces.FindIndex(SpaceHandle(space));
  Math::Swap(mSpaces[index], mSpaces[mSpaces.Size() - 1]);
  mSpaces.PopBack();
  mSpacesToDestroy.PushBack(space);
}

void Engine::DestroyQueuedCompositions()
{
  for(SpaceHandle space : mSpacesToDestroy)
    space.Delete();
  mSpacesToDestroy.Clear();
}

void Engine::InitializeCompositions(const CompositionInitializer& initializer)
{
  for(Space* space : mSpaces)
  {
    space->Initialize(initializer);
  }
}

void Engine::Update(float dt)
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
  DestroyQueuedCompositions();
}
