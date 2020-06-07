#include "Precompiled.hpp"

#include "Engine.hpp"

#include "Space.hpp"

//-------------------------------------------------------------------Engine
ZilchDefineType(Engine, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();
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
