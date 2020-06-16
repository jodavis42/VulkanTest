#pragma once

#include "ResourcesStandard.hpp"
#include "ResourceId.hpp"
#include "Zilch/Zilch.hpp"

class ResourceManager;
class ResourceLibrary;
class ResourceMetaFile;

//-------------------------------------------------------------------Resource
class Resource : public Zilch::IZilchObject
{
public:
  ZilchDeclareType(Resource, Zilch::TypeCopyMode::ReferenceType);

  virtual ~Resource() {}

  void Initialize(const ResourceMetaFile& resourceMeta);
  bool operator==(const Resource& rhs) const;
  size_t Hash() const;

  ResourceId mId;
  ResourceName mName;
  ResourcePath mPath;
  ResourceLibrary* mLibrary = nullptr;
  ResourceManager* mResourceManager = nullptr;
};
