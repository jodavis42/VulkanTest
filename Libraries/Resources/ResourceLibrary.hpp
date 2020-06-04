#pragma once

#include "ResourcesStandard.hpp"

#include "Resource.hpp"
#include "ResourceMetaFile.hpp"

struct ResourceExtensionManager;

class ResourceLibrary
{
public:
  void Load(ResourceExtensionManager& extensionManager);
  void Load(ResourceExtensionManager& extensionManager, const String& path, bool recursive);
  ResourceMetaFile LoadMetaFileForResource(const ResourcePath& path, const ResourceTypeName& resourceTypeName);

  String mLibraryName;
  String mLibraryPath;
  bool mRecursiveLoad = true;

  HashMap<ResourceId, ResourceMetaFile> mResourceIdMetaMap;
  HashMap<ResourcePath, ResourceId> mResourcePathToIdMap;
  HashMap<ResourceExtension, Array<ResourceId>> mExtensionsToMetaFilePaths;
};
