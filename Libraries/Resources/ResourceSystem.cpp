#include "Precompiled.hpp"

#include "ResourceSystem.hpp"
#include "ResourceManager.hpp"
#include "ResourceLibrary.hpp"

ZilchDefineType(ResourceSystem, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();
}

ResourceSystem::~ResourceSystem()
{
  DestroyAllManagers();
}

void ResourceSystem::Register(const ResourceTypeName& resourceTypeName, const ResourceManagerTypeName& resourceManagerTypeName, const ResourceManagerHandle& manager)
{
  mResourceToManagerTypeMap.InsertOrError(resourceTypeName, resourceManagerTypeName);
  mResourceManagerMap.InsertOrError(resourceManagerTypeName, manager);

  Array<ResourceExtension> extensions;
  manager->GetExtensions(extensions);
  for(const ResourceExtension& extension : extensions)
    mExtensionManager.Register(extension, resourceTypeName);
}

void ResourceSystem::DestroyManager(const ResourceManagerTypeName& managerTypeName)
{
  ResourceManager* manager = mResourceManagerMap.FindValue(managerTypeName, nullptr);
  if(manager != nullptr)
  {
    delete manager;
    mResourceManagerMap.Erase(managerTypeName);
  }
}

void ResourceSystem::DestroyAllManagers()
{
  for(ResourceManager* manager : mResourceManagerMap.Values())
  {
    delete manager;
  }
  mResourceManagerMap.Clear();
  mResourceToManagerTypeMap.Clear();
}

void ResourceSystem::LoadLibrary(const String& libraryName, const String& libraryPath, bool recursivelyLoad)
{
  ResourceLibrary* library = new ResourceLibrary();
  library->mLibraryName = libraryName;
  library->mLibraryPath = libraryPath;
  library->mRecursiveLoad = recursivelyLoad;
  library->Load(mExtensionManager);
  LoadLibrary(library);
}

void ResourceSystem::LoadLibrary(ResourceLibrary* library)
{
  library->mResourceSystem = this;
  mResourceLibraryGraph.PushLibrary(library);
  for(auto pair : library->mExtensionsToMetaFilePaths)
  {
    ResourceTypeName* resourceTypeName = mExtensionManager.Find(pair.first);
    if(resourceTypeName != nullptr)
    {
      ResourceManager* resourceManager = FindManagerBase(*resourceTypeName);
      if(resourceManager != nullptr)
      {
        for(const ResourceId& resourceId : pair.second)
        {
          ResourceMetaFile* metaFile = library->mResourceIdMetaMap.FindPointer(resourceId);
          resourceManager->LoadResource(*metaFile, library);
        }
      }
    }
  }
}

void ResourceSystem::ReloadLibraries()
{
  for(auto library : mResourceLibraryGraph.GetLibraries())
  {
    for(auto pair : library->mExtensionsToMetaFilePaths)
    {
      ResourceTypeName* resourceTypeName = mExtensionManager.Find(pair.first);
      if(resourceTypeName != nullptr)
      {
        ResourceManager* resourceManager = FindManagerBase(*resourceTypeName);
        if(resourceManager != nullptr)
        {
          for(const ResourceId& resourceId : pair.second)
          {
            ResourceMetaFile* metaFile = library->mResourceIdMetaMap.FindPointer(resourceId);
            resourceManager->ReLoadResource(*metaFile);
          }
        }
      }
    }
  }
}

ResourceSystem::ResourceManagerHandle ResourceSystem::FindManagerBase(const ResourceTypeName& typeName) const
{
  ResourceManagerTypeName managerTypeName = mResourceToManagerTypeMap.FindValue(typeName, ResourceManagerTypeName());
  return FindManagerBase(managerTypeName);
}

ResourceSystem::ResourceManagerHandle ResourceSystem::FindManagerBase(const ResourceManagerTypeName& managerTypeName) const
{
  return mResourceManagerMap.FindValue(managerTypeName, ResourceManagerHandle());
}

ResourceLibraryGraph* ResourceSystem::GetLibraryGraph()
{
  return &mResourceLibraryGraph;
}
