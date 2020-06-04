#pragma once

#include "ResourcesStandard.hpp"
#include "ResourceId.hpp"

class ResourceManager;
class ResourceLibrary;
class ResourceMetaFile;

class Resource
{
public:
  virtual ~Resource() {}

  void Initialize(const ResourceMetaFile& resourceMeta);

  ResourceId mId;
  ResourceName mName;
  ResourcePath mPath;
  ResourceLibrary* mLibrary = nullptr;
  ResourceManager* mResourceManager = nullptr;
};