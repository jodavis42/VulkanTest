#include "Precompiled.hpp"

#include "Space.hpp"
#include "Component.hpp"

#include "GameSession.hpp"
#include "Engine.hpp"

//-------------------------------------------------------------------Space
ZilchDefineType(Space, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindGetter(Game);
  ZilchBindGetter(Engine);
}

Space::Space()
{
  mSpace = this;
}

void Space::Add(Composition* composition)
{
  composition->mSpace = this;
  mCompositions.PushBack(composition);
}

void Space::QueueForDestruction(Composition* composition)
{
  size_t index = mCompositions.FindIndex(CompositionHandle(composition));
  Math::Swap(mCompositions[index], mCompositions[mCompositions.Size() - 1]);
  mCompositions.PopBack();
  mCompositionsToDestroy.PushBack(composition);
}

void Space::DestroyQueuedCompositions()
{
  for(CompositionHandle composition : mCompositionsToDestroy)
    composition.Delete();
  mCompositionsToDestroy.Clear();
}

void Space::InitializeCompositions(const CompositionInitializer& initializer)
{
  for(Composition* composition : mCompositions)
  {
    composition->Initialize(initializer);
  }
}

GameSession* Space::GetGame() const
{
  return mGame;
}

Engine* Space::GetEngine() const
{
  GameSession* game = GetGame();
  if(game == nullptr)
    return nullptr;
  return game->GetEngine();
}
