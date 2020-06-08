#include "Precompiled.hpp"

#include "Component.hpp"
#include "Composition.hpp"
#include "Space.hpp"
#include "Engine.hpp"

//-----------------------------------------------------------------------------Component
ZilchDefineType(Component, builder, type)
{
  type->HandleManager = ZilchManagerId(Zilch::PointerManager);
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindGetter(Owner);
  ZilchBindGetter(Space);
  ZilchBindGetter(Engine);
}

Composition* Component::GetOwner() const
{
  return mOwner;
}

Space* Component::GetSpace() const
{
  return GetOwner()->GetSpace();
}

Engine* Component::GetEngine() const
{
  Space* space = GetSpace();
  if(space == nullptr)
    return nullptr;
  return space->GetEngine();
}
