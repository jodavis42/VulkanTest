#pragma once

#include "ResourcesStandard.hpp"
#include "ResourceId.hpp"
#include "ResourceExtensionManager.hpp"
#include "ResourceLibraryGraph.hpp"

class ResourceManager;
class ResourceLibrary;

class ResourceSystem : public Zilch::EventHandler
{
public:
  using ResourceManagerHandle = Zilch::HandleOf<ResourceManager>;

  ZilchDeclareType(ResourceSystem, Zilch::TypeCopyMode::ReferenceType);
  ~ResourceSystem();

  void Register(const ResourceTypeName& resourceTypeName, const ResourceManagerTypeName& resourceManagerTypeName, const ResourceManagerHandle& manager);
  void DestroyManager(const ResourceManagerTypeName& managerTypeName);
  void DestroyAllManagers();

  void LoadLibrary(const String& libraryName, const String& libraryPath, bool recursivelyLoad = true);
  void LoadLibrary(ResourceLibrary* library);
  void ReloadLibraries();

  ResourceManagerHandle FindManagerBase(const ResourceTypeName& typeName) const;
  ResourceManagerHandle FindManagerBase(const ResourceManagerTypeName& managerTypeName) const;

  template <typename ResourceManagerType>
  ResourceManagerType* FindManager(const ResourceManagerTypeName& managerTypeName) const
  {
    return FindManagerBase(managerTypeName).Get<ResourceManagerType*>();
  }
  template <typename ResourceManagerType>
  ResourceManagerType* FindManager(const ResourceTypeName& typeName) const
  {
    return FindManagerBase(typeName).Get<ResourceManagerType*>();
  }

  ResourceLibraryGraph* GetLibraryGraph();

  ResourceExtensionManager mExtensionManager;
private:
  ResourceLibraryGraph mResourceLibraryGraph;
  HashMap<ResourceTypeName, ResourceManagerTypeName> mResourceToManagerTypeMap;
  HashMap<ResourceManagerTypeName, ResourceManagerHandle> mResourceManagerMap;
};

#define RegisterResourceManager(ResourceType, ResourceManagerType, managerInstance) \
  Register({#ResourceType}, {#ResourceManagerType}, managerInstance)
#define FindResourceManager(ResourceManagerType) FindManager<ResourceManagerType>(ResourceManagerTypeName{#ResourceManagerType})
#define FindResourceManagerForResource(ResourceType) FindManager<ResourceType>(ResourceTypeName{#ResourceType})
