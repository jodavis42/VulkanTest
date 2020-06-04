#pragma once

#include "ResourcesStandard.hpp"
#include "ResourceId.hpp"
#include "ResourceExtensionManager.hpp"

class ResourceManager;
class ResourceLibrary;

class ResourceSystem
{
public:
  ~ResourceSystem();

  void Register(const ResourceTypeName& resourceTypeName, const ResourceManagerTypeName& resourceManagerTypeName, ResourceManager* manager);
  void DestroyManager(const ResourceManagerTypeName& managerTypeName);
  void DestroyAllManagers();

  void LoadLibrary(const String& libraryName, const String& libraryPath, bool recursivelyLoad = true);
  void LoadLibrary(ResourceLibrary* library);
  void ReloadLibraries();

  ResourceManager* FindManagerBase(const ResourceTypeName& typeName) const;
  ResourceManager* FindManagerBase(const ResourceManagerTypeName& managerTypeName) const;

  template <typename ResourceManagerType>
  ResourceManagerType* FindManager(const ResourceManagerTypeName& managerTypeName) const
  {
    return static_cast<ResourceManagerType*>(FindManagerBase(managerTypeName));
  }
  template <typename ResourceManagerType>
  ResourceManagerType* FindManager(const ResourceTypeName& typeName) const
  {
    return static_cast<ResourceManagerType*>(FindManagerBase(typeName));
  }

  ResourceExtensionManager mExtensionManager;
private:
  Array<ResourceLibrary*> mLibraryStack;
  HashMap<ResourceTypeName, ResourceManagerTypeName> mResourceToManagerTypeMap;
  HashMap<ResourceManagerTypeName, ResourceManager*> mResourceManagerMap;
};

#define RegisterResourceManager(ResourceType, ResourceManagerType, managerInstance) \
  Register({#ResourceType}, {#ResourceManagerType}, managerInstance)
#define FindResourceManager(ResourceManagerType) FindManager<ResourceManagerType>(ResourceManagerTypeName{#ResourceManagerType})
#define FindResourceManagerForResource(ResourceType) FindManager<ResourceType>(ResourceTypeName{#ResourceType})
