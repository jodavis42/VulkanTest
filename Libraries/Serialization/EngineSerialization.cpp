#include "Precompiled.hpp"

#include "EngineSerialization.hpp"

#include "Utilities/JsonSerializers.hpp"
#include "ZilchHelpers/ZilchModule.hpp"
#include "Resources/ResourceSet.hpp"
#include "Resources/ResourceSystem.hpp"
#include "Engine/Component.hpp"
#include "Engine/Composition.hpp"
#include "Engine/Space.hpp"
#include "Engine/LevelManager.hpp"

void ParseResourceIdName(const String& resourceNameId, ResourceName& resourceName, ResourceId& resourceId)
{
  auto range = resourceNameId.All().FindRangeInclusive("{", "}");
  if(range.Empty())
  {
    resourceName = ResourceName{resourceNameId};
    resourceId = ResourceId::cInvalid;
    return;
  }

  String nameStr = resourceNameId.SubString(resourceNameId.Begin(), range.Begin());
  String idStr = resourceNameId.SubString(range.Begin() + 1, range.End() - 1);
  uint64 guid;
  Zero::ToValue(idStr, guid);
  resourceName = ResourceName{nameStr};
  resourceId = ResourceId{guid};
}

template <typename PropertyType>
void SerializeType(JsonLoader& loader, Zilch::Any& result)
{
  PropertyType data;
  loader.SerializePrimitive(data);
  result = data;
}

template <typename PropertyType, size_t Count>
void SerializeArrayType(JsonLoader& loader, Zilch::Any& result)
{
  PropertyType data;
  LoadArray<PropertyType, Count>(loader, data);
  result = data;
}

bool LoadProperty(SerializerContext& context, Zilch::Type* propertyType, const String& propertyName, Zilch::Any& result)
{
  JsonLoader& loader = *context.mLoader;
  if(!loader.BeginMember(propertyName))
    return false;

  bool returnValue = true;
  if(propertyType == ZilchTypeId(bool))
    SerializeType<bool>(loader, result);
  if(propertyType == ZilchTypeId(int))
    SerializeType<int>(loader, result);
  else if(propertyType == ZilchTypeId(float))
    SerializeType<float>(loader, result);
  else if(propertyType == ZilchTypeId(Vec2))
    SerializeArrayType<Vec2, 2>(loader, result);
  else if(propertyType == ZilchTypeId(Vec3))
    SerializeArrayType<Vec3, 3>(loader, result);
  else if(propertyType == ZilchTypeId(Vec4))
    SerializeArrayType<Vec4, 4>(loader, result);
  else if(propertyType == ZilchTypeId(Quaternion))
    SerializeArrayType<Quaternion, 4>(loader, result);
  else if(propertyType == ZilchTypeId(String))
    SerializeType<String>(loader, result);
  else if(propertyType->IsA(ZilchTypeId(Zilch::Enum)))
  {
    String enumValue;
    loader.SerializePrimitive(enumValue);

    Zilch::BoundType* propertyBoundType = Zilch::BoundType::GetBoundType(propertyType);
    Zilch::Property* zilchPropertyValue = propertyBoundType->FindProperty(enumValue, Zilch::FindMemberOptions::Static);
    if(zilchPropertyValue != nullptr)
      result = zilchPropertyValue->Get->Invoke(Zilch::Any(), nullptr);
  }
  else if(propertyType->IsA(ZilchTypeId(Resource)))
  {
    Zilch::BoundType* propertyBoundType = Zilch::BoundType::GetBoundType(propertyType);
    ResourceManager* manager = context.mResourceSystem->FindManagerBase(ResourceTypeName{propertyBoundType->Name});
    if(manager != nullptr)
    {
      String resourceNameId;
      loader.SerializePrimitive(resourceNameId);
      ResourceName resourceName;
      ResourceId resourceId;
      ParseResourceIdName(resourceNameId, resourceName, resourceId);

      Resource* resource = nullptr;
      if(resourceId != ResourceId::cInvalid)
        resource = manager->FindResourceBase(resourceId);
      if(resource == nullptr && !resourceName.Empty())
        resource = manager->FindResourceBase(resourceName);
      result = resource;
    }
  }
  else if(propertyType->IsA(ZilchTypeId(ResourceSet)))
  {
    Zilch::BoundType* propertyBoundType = Zilch::BoundType::GetBoundType(propertyType);
    Zilch::ExceptionReport report;
    Zilch::ExecutableState* state = Zilch::ExecutableState::CallingState;
    result = state->AllocateDefaultConstructedHeapObject(propertyBoundType, report, Zilch::HeapFlags::ReferenceCounted);
    ResourceSet* resourceSet = result.Get<ResourceSet*>();

    size_t count = 0;
    loader.BeginArray(count);
    for(size_t i = 0; i < count; ++i)
    {
      loader.BeginArrayItem(i);
      String resourceName;
      loader.SerializePrimitive(resourceName);
      resourceSet->AddResource(context.mResourceSystem, ResourceName{resourceName});
      loader.EndArrayItem();
    }
  }
  else
    returnValue = false;

  loader.EndMember();
  return true;
}

bool LoadProperty(SerializerContext& context, Zilch::Property* zilchProperty, Zilch::Handle objectInstanceHandle)
{
  JsonLoader& loader = *context.mLoader;
  Zilch::Attribute* propertyAttribute = zilchProperty->HasAttribute(Zilch::PropertyAttribute);
  if(propertyAttribute == nullptr)
    return false;

  Zilch::Type* propertyType = zilchProperty->PropertyType;
  String propertyName = zilchProperty->Name;
  if(Zilch::AttributeParameter* nameAttributeParam = propertyAttribute->HasAttributeParameter("Name"))
    propertyName = nameAttributeParam->StringValue;

  Zilch::Function* getter = zilchProperty->Get;
  Zilch::Function* setter = zilchProperty->Set;
  Zilch::ArrayClass<Zilch::Any> setArguments;
  Zilch::Any setValue;

  if(!LoadProperty(context, propertyType, propertyName, setValue))
    return false;

  setArguments.NativeArray.PushBack(setValue);
  setter->Invoke(objectInstanceHandle, &setArguments);

  return true;
}

bool LoadComponent(SerializerContext& context, const String& componentName, Composition* compositionOwner)
{
  JsonLoader& loader = *context.mLoader;
  if(componentName == "Name")
  {
    loader.SerializePrimitive(compositionOwner->mName);
    loader.EndMember();
    return false;
  }

  Zilch::ExceptionReport report;
  Zilch::ExecutableState* state = Zilch::ExecutableState::CallingState;
  Zilch::BoundType* boundType = context.mModule->FindType(componentName);
  if(boundType == nullptr)
  {
    Zilch::Console::WriteLine("Failed to find bound type '%s' when serializing object '%s'", componentName.c_str(), compositionOwner->mName.c_str());
    loader.EndMember();
    return false;
  }

  Zilch::Handle preconstructedObject = state->AllocateDefaultConstructedHeapObject(boundType, report, Zilch::HeapFlags::ReferenceCounted);
  Component* component = preconstructedObject.Get<Component*>();
  compositionOwner->AddComponent(component);
  for(auto range = boundType->GetProperties(); !range.Empty(); range.PopFront())
  {
    LoadProperty(context, range.Front(), preconstructedObject);
  }
  loader.EndMember();
  return true;
}

bool CloneComponent(SerializerContext& context, Component& oldComponent, Zilch::HandleOf<Component>& newComponent)
{
  Zilch::ExceptionReport report;
  Zilch::ExecutableState* state = Zilch::ExecutableState::CallingState;
  Zilch::BoundType* oldBoundType = ZilchVirtualTypeId(&oldComponent);
  Zilch::BoundType* newBoundType = context.mModule->FindType(oldBoundType->Name);
  Zilch::Handle preconstructedObject = state->AllocateDefaultConstructedHeapObject(newBoundType, report, Zilch::HeapFlags::ReferenceCounted);
  newComponent = preconstructedObject.Get<Component*>();
  for(auto range = newBoundType->GetProperties(); !range.Empty(); range.PopFront())
  {
    Zilch::Property* newProperty = range.Front();
    if(newProperty->Set == nullptr)
      continue;
    Zilch::Property* oldProperty = oldBoundType->FindProperty(newProperty->Name, Zilch::FindMemberOptions::None);
    if(oldProperty == nullptr)
      continue;

    Zilch::Any getValue = oldProperty->Get->Invoke(&oldComponent, nullptr);
    Zilch::ArrayClass<Zilch::Any> setArgs;
    setArgs.NativeArray.PushBack(getValue);
    newProperty->Set->Invoke(newComponent, &setArgs);
  }
  return true;
}

bool LoadComposition(SerializerContext& context, const String& path, Composition* composition)
{
  if(!context.mLoader->LoadFromFile(path))
    return false;
  return LoadComposition(context, composition);
}

bool LoadComposition(SerializerContext& context, Composition* composition)
{
  JsonLoader& loader = *context.mLoader;
  size_t componentCount;
  if(!loader.BeginMembers(componentCount))
    return false;

  for(size_t i = 0; i < componentCount; ++i)
  {
    String componentName;
    if(loader.BeginMember(i, componentName))
    {
      LoadComponent(context, componentName, composition);
    }
  }
  return true;
}

bool LoadLevel(SerializerContext& context, Level* level, Space* space)
{
  JsonLoader& loader = *context.mLoader;
  if(!loader.LoadFromFile(level->mPath))
    return false;

  size_t objCount;
  loader.BeginArray(objCount);
  for(size_t objIndex = 0; objIndex < objCount; ++objIndex)
  {
    loader.BeginArrayItem(objIndex);

    CompositionHandle composition = ZilchAllocate(Composition);
    LoadComposition(context, composition);
    space->Add(composition);

    loader.EndArrayItem();
  }
  return true;
}
