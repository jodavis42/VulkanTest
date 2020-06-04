#pragma once

#include "ResourcesStandard.hpp"

#include "ResourceId.hpp"

class ResourceMetaFile
{
public:
  ResourceTypeName mResourceTypeName;
  ResourceName mName;
  ResourcePath mResourcePath;
  ResourceId mId;
};
