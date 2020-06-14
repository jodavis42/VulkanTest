#include "Precompiled.hpp"

#include "Space.hpp"
#include "Component.hpp"

#include "ArchetypeManager.hpp"
#include "CompositionInitializer.hpp"
#include "Engine.hpp"
#include "GameSession.hpp"
#include "IApplication.hpp"
#include "Transform.hpp"

//-------------------------------------------------------------------Space
ZilchDefineType(Space, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindOverloadedMethod(Create, ZilchInstanceOverload(CompositionHandle, Archetype&));
  ZilchBindOverloadedMethod(Create, ZilchInstanceOverload(CompositionHandle, Archetype&, CompositionCreationContext&));
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

CompositionHandle Space::Create(Archetype& archetype)
{
  CompositionHandle composition = CreateNoInit(archetype);
  composition->Initialize(CompositionInitializer());
  return composition;
}

CompositionHandle Space::Create(Archetype& archetype, CompositionCreationContext& creationContext)
{
  CompositionHandle composition = CreateNoInit(archetype);
  Transform* transform = composition->Has<Transform>();
  transform->mScale = creationContext.mScale;
  transform->mRotation = creationContext.mRotation;
  transform->mTranslation = creationContext.mTranslation;
  composition->Initialize(CompositionInitializer());
  return composition;
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

CompositionHandle Space::CreateNoInit(Archetype& archetype)
{
  Engine* engine = GetEngine();
  CompositionHandle composition = engine->mApplication->CreateComposition(archetype);
  Add(composition);
  return composition;
}
