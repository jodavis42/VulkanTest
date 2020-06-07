#include "Precompiled.hpp"

#include "Composition.hpp"

#include "CompositionInitializer.hpp"
#include "Component.hpp"
#include "Space.hpp"

//-------------------------------------------------------------------Composition
ZilchDefineType(Composition, builder, type)
{
  type->HandleManager = ZilchManagerId(Zilch::PointerManager);
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindGetter(Space);
  ZilchBindOverloadedMethod(FindComponent, ZilchInstanceOverload(Component*, const String&));
  ZilchBindMethod(Destroy);
}

Composition::Composition()
{
}

Composition::~Composition()
{
  DestroyAllComponents();
}

void Composition::Initialize(const CompositionInitializer& initializer)
{
  for(Component* component : mComponents)
  {
    component->Initialize(initializer);
  }
}

void Composition::AddComponent(Component* component)
{
  component->mOwner = this;
  Zilch::BoundType* boundType = ZilchVirtualTypeId(component);
  mComponents.PushBack(component);
  mComponentMap[boundType] = component;
}

Component* Composition::FindComponent(const Zilch::BoundType* boundType)
{
  return mComponentMap.FindValue(boundType, nullptr);
}

Component* Composition::FindComponent(const String& typeName)
{
  for(auto range = Zilch::ExecutableState::CallingState->Dependencies.All(); !range.Empty(); range.PopFront())
  {
    Zilch::BoundType* boundType = range.Front()->BoundTypes.FindValue(typeName, nullptr);
    if(boundType != nullptr)
      return FindComponent(boundType);
  }
  return nullptr;
}

void Composition::Destroy()
{
  GetSpace()->QueueForDestruction(this);
}

void Composition::DestroyAllComponents()
{
  for(ComponentHandle component : mComponents)
    component->OnDestroy();
  for(ComponentHandle component : mComponents)
    component.Delete();
  mComponents.Clear();
  mComponentMap.Clear();
}

Space* Composition::GetSpace() const
{
  return mSpace;
}
