#include "Precompiled.hpp"

#include "ResourceManager.hpp"

//-----------------------------------------------------------------------------ResourceManager
ZilchDefineType(ResourceManager, builder, type)
{
  type->HandleManager = ZilchManagerId(Zilch::PointerManager);
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  builder.AddSendsEvent(type, Events::ResourceLoaded, ZilchTypeId(ResourceLoadEvent));
  builder.AddSendsEvent(type, Events::ResourceReLoaded, ZilchTypeId(ResourceLoadEvent));
}

ResourceManager::~ResourceManager()
{
  DestroyAllResources();
}

void ResourceManager::DestroyAllResources()
{
  Array<ResourceId> resourceIds;
  resourceIds.Append(mResourceIdToResource.Keys());
  for(const ResourceId& id : resourceIds)
    DestroyResource(id);
}

void ResourceManager::DestroyResource(const ResourceId& id)
{
  ResourceHandle resource = mResourceIdToResource.FindValue(id, nullptr);
  if(resource != nullptr)
  {
    mResourceNameToId.Erase(resource->mName);
    mResourcePathToId.Erase(resource->mPath);
    mResourceIdToResource.Erase(id);
    resource.Delete();
  }
}

void ResourceManager::RegisterResource(ResourceHandle resource)
{
  mResourceIdToResource[resource->mId] = resource;
  mResourceNameToId[resource->mName] = resource->mId;
  mResourcePathToId[resource->mPath] = resource->mId;
}

ResourceId ResourceManager::FindId(const ResourceName& name) const
{
  return mResourceNameToId.FindValue(name, ResourceId::cInvalid);
}

ResourceId ResourceManager::FindId(const ResourcePath& path) const
{
  return mResourcePathToId.FindValue(path, ResourceId::cInvalid);
}

ResourceManager::ResourceHandle ResourceManager::FindResourceBase(const ResourceName& name) const
{
  return FindResourceBase(FindId(name));
}

ResourceManager::ResourceHandle ResourceManager::FindResourceBase(const ResourcePath& path) const
{
  return FindResourceBase(FindId(path));
}

ResourceManager::ResourceHandle ResourceManager::FindResourceBase(const ResourceId& id) const
{
  return mResourceIdToResource.FindValue(id, nullptr);
}
