#include "Precompiled.hpp"

#include "Resource.hpp"
#include "ResourceMetaFile.hpp"

void Resource::Initialize(const ResourceMetaFile& resourceMeta)
{
  mId = resourceMeta.mId;
  mName = resourceMeta.mName;
  mPath = resourceMeta.mResourcePath;
}
