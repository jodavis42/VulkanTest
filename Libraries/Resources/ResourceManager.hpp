#pragma once

#include "ResourcesStandard.hpp"
#include "Zilch/Zilch.hpp"
#include "ResourceId.hpp"
#include "Resource.hpp"
#include "ResourceMetaFile.hpp"
#include "ResourceEvents.hpp"
#include "ResourceExtensions.hpp"

class ResourceManager : public Zilch::EventHandler
{
public:
  using ResourceHandle = Zilch::HandleOf<Resource>;
  ZilchDeclareType(ResourceManager, Zilch::TypeCopyMode::ReferenceType);

  virtual ~ResourceManager();

  void DestroyAllResources();
  void DestroyResource(const ResourceId& id);

  void RegisterResource(Zilch::HandleOf<Resource> resource);
  ResourceId FindId(const ResourceName& name) const;
  ResourceId FindId(const ResourcePath& path) const;
  ResourceHandle FindResourceBase(const ResourceName& name) const;
  ResourceHandle FindResourceBase(const ResourcePath& path) const;
  ResourceHandle FindResourceBase(const ResourceId& id) const;

  virtual void GetExtensions(Array<ResourceExtension>& extensions) const {}
  virtual bool LoadResource(const ResourceMetaFile& resourceMeta, ResourceLibrary* library) { return false; }
  virtual bool ReLoadResource(const ResourceMetaFile& resourceMeta) { return false; }

protected:
  using ResourceIdMap = HashMap <ResourceId, ResourceHandle>;
  HashMap<ResourcePath, ResourceId> mResourcePathToId;
  HashMap<ResourceName, ResourceId> mResourceNameToId;
  ResourceIdMap mResourceIdToResource;
};

template <typename ResourceType>
class ResourceManagerTyped : public ResourceManager
{
public:
  struct ResourceRange
  {
    typedef ResourceRange SelfType;
    typedef ResourceType* FrontResult;
    using RangeType = ResourceIdMap::valuerange;
    RangeType mRange;

    ResourceRange(RangeType range) : mRange(range) {}
    FrontResult Front() { return mRange.Front().Get<ResourceType*>(); }
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
    return FindResourceBase(name).Get<ResourceType*>();
  }

  ResourceType* FindResource(const ResourcePath& path) const
  {
    return FindResourceBase(path).Get<ResourceType*>();
  }

  ResourceType* FindResource(const ResourceId& id) const
  {
    return FindResourceBase(id).Get<ResourceType*>();
  }

  virtual bool LoadResource(const ResourceMetaFile& resourceMeta, ResourceLibrary* library) override
  {
    Zilch::HandleOf<ResourceType> resource = ZilchAllocate(ResourceType);
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
