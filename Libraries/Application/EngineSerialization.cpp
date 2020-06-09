#include "Precompiled.hpp"

#include "EngineSerialization.hpp"

#include "Utilities/JsonSerializers.hpp"
#include "Engine/Component.hpp"
#include "Engine/Composition.hpp"
#include "Engine/Space.hpp"
#include "Engine/LevelManager.hpp"
#include "ZilchScript/ZilchScriptLibrary.hpp"

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

bool LoadProperty(JsonLoader& loader, Zilch::Type* propertyType, const String& propertyName, Zilch::Any& result)
{
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
  else
    returnValue = false;

  loader.EndMember();
  return true;
}

bool LoadProperty(JsonLoader& loader, Zilch::Property* zilchProperty, Zilch::Handle objectInstanceHandle)
{
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

  if(!LoadProperty(loader, propertyType, propertyName, setValue))
    return false;

  setArguments.NativeArray.PushBack(setValue);
  setter->Invoke(objectInstanceHandle, &setArguments);

  return true;
}

bool LoadComponent(ZilchScriptModule* module, JsonLoader& loader, const String& componentName, Composition* compositionOwner)
{
  if(componentName == "Name")
  {
    loader.SerializePrimitive(compositionOwner->mName);
    loader.EndMember();
    return false;
  }

  Zilch::ExceptionReport report;
  Zilch::ExecutableState* state = Zilch::ExecutableState::CallingState;
  Zilch::BoundType* boundType = module->FindType(componentName);
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
    LoadProperty(loader, range.Front(), preconstructedObject);
  }
  loader.EndMember();
  return true;
}

bool LoadComposition(ZilchScriptModule* module, const String& path, Composition* composition)
{
  JsonLoader loader;
  if(!loader.LoadFromFile(path))
    return false;
  return LoadComposition(module, loader, composition);
}

bool LoadComposition(ZilchScriptModule* module, JsonLoader& loader, Composition* composition)
{
  size_t componentCount;
  if(!loader.BeginMembers(componentCount))
    return false;

  for(size_t i = 0; i < componentCount; ++i)
  {
    String componentName;
    if(loader.BeginMember(i, componentName))
    {
      LoadComponent(module, loader, componentName, composition);
    }
  }
  return true;
}

bool LoadLevel(ZilchScriptModule* module, Level* level, Space* space)
{
  JsonLoader loader;
  if(!loader.LoadFromFile(level->mPath))
    return false;

  size_t objCount;
  loader.BeginArray(objCount);
  for(size_t objIndex = 0; objIndex < objCount; ++objIndex)
  {
    loader.BeginArrayItem(objIndex);

    CompositionHandle composition = ZilchAllocate(Composition);
    LoadComposition(module, loader, composition);
    space->Add(composition);

    loader.EndArrayItem();
  }
  return true;
}
