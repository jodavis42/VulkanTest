#pragma once

#include "ResourcesStandard.hpp"
#include "Zilch/Zilch.hpp"
#include "ResourceId.hpp"
#include "Resource.hpp"
#include "ResourceManager.hpp"
#include "ResourceSystem.hpp"

//-------------------------------------------------------------------ResourceSet
class ResourceSet
{
public:
  ZilchDeclareType(ResourceSet, Zilch::TypeCopyMode::ReferenceType);
  using ResourceHandle = Zilch::HandleOf<Resource>;

  virtual void AddResource(const ResourceHandle& resource) {}
  virtual void AddResource(ResourceSystem* resourceSystem, const ResourceName& resourceName) {}
  virtual void RemoveResource(const ResourceHandle& resource) {}
  virtual void RemoveResource(ResourceSystem* resourceSystem, const ResourceName& resourceName) {}
  virtual bool ContainsResource(const ResourceHandle& resource) { return false; }
  virtual void Clear() {}
};

//-------------------------------------------------------------------TypedResourcesSet
template <typename ResourceType>
class TypedResourceSet : public ResourceSet
{
public:
  using ResourceTypeHandle = Zilch::HandleOf<ResourceType>;

  static ResourceTypeName GetResourceTypeName()
  {
    Zilch::BoundType* boundType = ZilchTypeId(ResourceType);
    return ResourceTypeName{boundType->Name};
  }

  virtual void AddResource(const ResourceHandle& resource) override
  {
    Add(resource);
  }

  virtual void AddResource(ResourceSystem* resourceSystem, const ResourceName& resourceName) override
  {
    Resource* resource = FindResource(resourceSystem, resourceName);
    if(resource != nullptr)
      AddResource(*resource);
  }

  virtual void Add(const ResourceTypeHandle& resource)
  {
    mSet.Insert(resource);
  }

  virtual void RemoveResource(const ResourceHandle& resource) override
  {
    Remove(resource);
  }

  virtual void RemoveResource(ResourceSystem* resourceSystem, const ResourceName& resourceName) override
  {
    Resource* resource = FindResource(resourceSystem, resourceName);
    if(resource != nullptr)
      RemoveResource(*resource);
  }

  virtual void Remove(const ResourceTypeHandle& resource)
  {
    mSet.Erase(resource);
  }

  virtual bool ContainsResource(const ResourceHandle& resource) override
  {
    return Contains(resource);
  }

  virtual bool Contains(const ResourceTypeHandle& resource) const
  {
    return mSet.Contains(resource);
  }

  virtual void Clear() override
  {
    mSet.Clear();
  }

private:
  ResourceHandle FindResource(ResourceSystem* resourceSystem, const ResourceName& resourceName)
  {
    ResourceManager* resourceManager = resourceSystem->FindManagerBase(GetResourceTypeName());
    return resourceManager->FindResourceBase(resourceName);
  }
  
  Zero::HashSet<ResourceTypeHandle, Zero::HashPolicy<Resource>> mSet;
};
