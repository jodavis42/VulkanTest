#include "Precompiled.hpp"

#include "EngineZilchStaticLibrary.hpp"
#include "Component.hpp"
#include "Composition.hpp"
#include "Engine.hpp"
#include "UpdateEvent.hpp"
#include "TimeSpace.hpp"
#include "Transform.hpp"
#include "Space.hpp"

// Get a component from a Composition in zilch
void GetComponentExtensionProperty(Zilch::Call& call, Zilch::ExceptionReport& report)
{
  Zilch::Function* currentFunction = call.GetFunction();
  Zilch::BoundType* boundType = Zilch::Type::DynamicCast<Zilch::BoundType*>(currentFunction->FunctionType->Return);

  Zilch::Handle& selfHandle = call.GetHandle(Zilch::Call::This);
  Composition* composition = selfHandle.Get<Composition*>();

  Zilch::Handle componentInstance = composition->FindComponent(boundType);
  call.Set(Zilch::Call::Return, componentInstance);
}

void AddComponentExtensions(Zilch::LibraryBuilder& builder, Zilch::BoundType* boundType)
{
  if(boundType->IsA(ZilchTypeId(Component)))
    builder.AddExtensionGetterSetter(ZilchTypeId(Composition), boundType->Name, boundType, nullptr, GetComponentExtensionProperty, Zilch::MemberOptions::None);
}

void AddNativeLibraryExtensions(Zilch::LibraryBuilder& builder)
{
  for(auto range = builder.BoundTypes.Values(); !range.Empty(); range.PopFront())
    AddComponentExtensions(builder, range.Front());
}

ZilchDefineStaticLibrary(EngineStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(Component);
  ZilchInitializeType(Composition);
  ZilchInitializeType(Space);
  ZilchInitializeType(Engine);
  ZilchInitializeType(TimeSpace);
  ZilchInitializeType(Transform);
  ZilchInitializeType(UpdateEvent);

  AddNativeLibraryExtensions(builder);
}
