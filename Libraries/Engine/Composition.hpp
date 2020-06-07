#pragma once

#include "EngineStandard.hpp"
#include "Zilch/Zilch.hpp"

class Component;
class CompositionInitializer;
class Space;

//-------------------------------------------------------------------Composition
class Composition : public Zilch::EventHandler
{
public:
  ZilchDeclareType(Composition, Zilch::TypeCopyMode::ReferenceType);

  Composition();
  virtual ~Composition();
  virtual void Initialize(const CompositionInitializer& initializer);

  virtual void AddComponent(Component* component);
  Component* FindComponent(const Zilch::BoundType* boundType);
  Component* FindComponent(const String& typeName);
  void Destroy();
  void DestroyAllComponents();

  template <typename ComponentType>
  ComponentType* Has()
  {
    Zilch::BoundType* boundType = ZilchTypeId(ComponentType);
    return static_cast<ComponentType*>(FindComponent(boundType));
  }

  Space* GetSpace() const;

  using ComponentHandle = Zilch::HandleOf<Component>;
  HashMap<const Zilch::BoundType*, ComponentHandle> mComponentMap;
  Array<ComponentHandle> mComponents;
  String mName;
  Space* mSpace = nullptr;
};

using CompositionHandle = Zilch::HandleOf<Composition>;
