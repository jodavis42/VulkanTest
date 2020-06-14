#include "Precompiled.hpp"

#include "ResourceExtensions.hpp"

#include "ZilchHelpers/ZilchModule.hpp"
#include "ZilchHelpers/ZilchCallingStateSingleton.hpp"
#include "ResourceManager.hpp"
#include "ResourceSystem.hpp"
#include "ResourceLibrary.hpp"

void GetResourceExtensionProperty(Zilch::Call& call, Zilch::ExceptionReport& report)
{
  Zilch::Function* currentFunction = call.GetFunction();
  Zilch::Property* zilchProperty = currentFunction->OwningProperty;
  Zilch::BoundType* boundType = Zilch::Type::DynamicCast<Zilch::BoundType*>(currentFunction->FunctionType->Return);
  ResourceManager* manager = (ResourceManager*)zilchProperty->UserData;
  
  Zilch::HandleOf<Resource> result = manager->FindResourceBase(ResourceName{zilchProperty->Name});
  result.StoredType = boundType;
  call.Set(Zilch::Call::Return, result);
}

void FindResourceExtensionMethod(Zilch::Call& call, Zilch::ExceptionReport& report)
{
  Zilch::Function* currentFunction = call.GetFunction();
  Zilch::BoundType* boundType = Zilch::Type::DynamicCast<Zilch::BoundType*>(currentFunction->FunctionType->Return);
  ZilchCallingStateSingleton* stateData = (ZilchCallingStateSingleton*)Zilch::ExecutableState::CallingState->UserData;
  ResourceSystem* resourceSystem = (ResourceSystem*)stateData->mResourceSystem;
  ResourceManager* manager = resourceSystem->FindManagerBase(ResourceTypeName{currentFunction->Owner->Name});

  String resourceName = call.Get<String>(0);
  Zilch::HandleOf<Resource> result = manager->FindResourceBase(ResourceName{resourceName});
  call.Set(Zilch::Call::Return, result);
}

void AddResourcePropertyExtension(Zilch::LibraryBuilder& builder, ResourceSystem* resourceSystem, Zilch::BoundType* resourceType, const ResourceName& resourceName)
{
  ResourceManager* manager = resourceSystem->FindManagerBase(ResourceTypeName{resourceType->Name});
  ErrorIf(manager == nullptr, "Failed to find resource manager for resource '%s : %s", resourceName.c_str(), resourceType->Name.c_str());
  Zilch::GetterSetter* getSet = builder.AddExtensionGetterSetter(resourceType, resourceName, resourceType, nullptr, GetResourceExtensionProperty, Zilch::MemberOptions::Static);
  getSet->UserData = manager;
  getSet->Add(manager);
}

void AddResourcePropertyExtensions(Zilch::LibraryBuilder& builder, ZilchModule* dependencies, ResourceLibrary* library)
{
  ResourceSystem* resourceSystem = library->mResourceSystem;
  for(auto pair : library->mResourceIdMetaMap)
  {
    ResourceManager* manager = resourceSystem->FindManagerBase(pair.second.mResourceTypeName);
    Zilch::BoundType* resourceType = dependencies->FindType(pair.second.mResourceTypeName);
    AddResourcePropertyExtension(builder, resourceSystem, resourceType, pair.second.mName);
  }
}

void AddResourceExtensions(Zilch::LibraryBuilder& builder, ResourceSystem* resourceSystem, Zilch::BoundType* resourceType)
{
  if(resourceType->IsA(ZilchTypeId(Resource)) == false)
    return;

  Zilch::ParameterArray findParameters;
  findParameters.PushBack(ZilchTypeId(String));
  Zilch::Function* function = builder.AddExtensionFunction(resourceType, "Find", FindResourceExtensionMethod, findParameters, resourceType, Zilch::MemberOptions::Static);
}

void AddResourceExtensions(Zilch::LibraryBuilder& builder)
{
  for(auto range = builder.BoundTypes.Values(); !range.Empty(); range.PopFront())
  {
    Zilch::BoundType* boundType = range.Front();
    AddResourceExtensions(builder, nullptr, boundType);
  }
}
