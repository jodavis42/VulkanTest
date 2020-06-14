#pragma once

#include "ResourcesStandard.hpp"

#include "Resource.hpp"
#include "ResourceMetaFile.hpp"

class ResourceSystem;
struct ResourceExtensionManager;

class ResourceLibrary : public Zilch::EventHandler
{
public:
  struct ResourceIdRangeOfResourceType
  {
    using RangeType = HashMap<ResourceId, ResourceMetaFile>::range;
    using FrontType = ResourceId;
    using SelfType = ResourceIdRangeOfResourceType;
    ResourceTypeName mResourceTypeName;
    RangeType mRange;

    ResourceIdRangeOfResourceType(const ResourceTypeName& resouceTypeName, RangeType range);
    bool Empty();
    FrontType Front();
    void PopFront();

    // C++ iterator/range interface
    SelfType begin() { return *this; }
    SelfType end();
    bool operator==(const SelfType& rhs) const;
    bool operator!=(const SelfType& rhs) const { return !(*this == rhs); }
    SelfType& operator++() { PopFront();  return *this; }
    FrontType operator*() { return Front(); }
  };

  void Load(ResourceExtensionManager& extensionManager);
  void Load(ResourceExtensionManager& extensionManager, const String& path, bool recursive);
  ResourceMetaFile LoadMetaFileForResource(const ResourcePath& path, const ResourceTypeName& resourceTypeName);
  ResourceIdRangeOfResourceType AllResourcesOfType(const ResourceTypeName& resourceTypeName);

  String mLibraryName;
  String mLibraryPath;
  bool mRecursiveLoad = true;

  ResourceSystem* mResourceSystem = nullptr;
  HashMap<ResourceId, ResourceMetaFile> mResourceIdMetaMap;
  HashMap<ResourcePath, ResourceId> mResourcePathToIdMap;
  HashMap<ResourceExtension, Array<ResourceId>> mExtensionsToMetaFilePaths;
};
