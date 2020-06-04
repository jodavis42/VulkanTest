#pragma once

#include "ResourcesStandard.hpp"
#include "Zilch/Zilch.hpp"
#include "ResourceId.hpp"
#include "Resource.hpp"
#include "ResourceMetaFile.hpp"
#include "ResourceEvents.hpp"

class ResourceManager : public Zilch::EventHandler
{
public:
  ZilchDeclareType(ResourceManager, Zilch::TypeCopyMode::ReferenceType);

  virtual ~ResourceManager();

  void DestroyAllResources();
  void DestroyResource(const ResourceId& id);

  void RegisterResource(Resource* resource);
  ResourceId FindId(const ResourceName& name) const;
  ResourceId FindId(const ResourcePath& path) const;
  Resource* FindResourceBase(const ResourceName& name) const;
  Resource* FindResourceBase(const ResourcePath& path) const;
  Resource* FindResourceBase(const ResourceId& id) const;

  virtual void GetExtensions(Array<ResourceExtension>& extensions) const {}
  virtual bool LoadResource(const ResourceMetaFile& resourceMeta, ResourceLibrary* library) { return false; }
  virtual bool ReLoadResource(const ResourceMetaFile& resourceMeta) { return false; }

protected:
  HashMap<ResourcePath, ResourceId> mResourcePathToId;
  HashMap<ResourceName, ResourceId> mResourceNameToId;
  HashMap<ResourceId, Resource*> mResourceIdToResource;
};

template <typename ResourceType>
class ResourceManagerTyped : public ResourceManager
{
public:
  struct ResourceRange
  {
    typedef ResourceRange SelfType;
    typedef ResourceType* FrontResult;
    using RangeType = HashMap<ResourceId, Resource*>::valuerange;
    RangeType mRange;

    ResourceRange(RangeType range) : mRange(range) {}
    FrontResult Front() { return static_cast<ResourceType*>(mRange.Front()); }
    void PopFront() { mRange.PopFront(); }
    bool Empty() const { mRange.Empty(); }

    // C++ iterator/range interface
    SelfType begin() { return *this; }
    SelfType end() { return SelfType(mRange.end()); }
    bool operator==(const SelfType& rhs) const { return mRange == rhs.mRange; }
    bool operator!=(const SelfType& rhs) const { return mRange != rhs.mRange; }
    SelfType& operator++() { PopFront(); return *this; }
    FrontResult operator*() { return Front(); }
  };

  ResourceRange Resources()
  {
    return ResourceRange(mResourceIdToResource.Values());
  }

  ResourceType* FindResource(const ResourceName& name) const
  {
    return static_cast<ResourceType*>(FindResourceBase(name));
  }

  ResourceType* FindResource(const ResourcePath& path) const
  {
    return static_cast<ResourceType*>(FindResourceBase(path));
  }

  ResourceType* FindResource(const ResourceId& id) const
  {
    return static_cast<ResourceType*>(FindResourceBase(id));
  }

  virtual bool LoadResource(const ResourceMetaFile& resourceMeta, ResourceLibrary* library) override
  {
    ResourceType* resource = new ResourceType();
    resource->Initialize(resourceMeta);
    resource->mLibrary = library;
    resource->mResourceManager = this;

    if(!OnLoadResource(resourceMeta, resource))
    {
      delete resource;
      return false;
    }

    RegisterResource(resource);

    ResourceLoadEvent toSend;
    toSend.EventName = Events::ResourceLoaded;
    toSend.mResource = resource;
    Zilch::EventSend(this, toSend.EventName, &toSend);
    return true;
  }

  virtual bool ReLoadResource(const ResourceMetaFile& resourceMeta) override
  {
    ResourceType* resource = FindResource(resourceMeta.mId);
    if(resource != nullptr)
    {
      if(OnReLoadResource(resourceMeta, resource))
      {
        ResourceLoadEvent toSend;
        toSend.EventName = Events::ResourceReLoaded;
        toSend.mResource = resource;
        Zilch::EventSend(this, toSend.EventName, &toSend);
        return true;
      }
    }
    return false;
  }

  virtual bool OnLoadResource(const ResourceMetaFile& resourceMeta, ResourceType* resource) { return false; }
  virtual bool OnReLoadResource(const ResourceMetaFile& resourceMeta, ResourceType* resource) { return false; }
};
