#include "Precompiled.hpp"

#include "Resource.hpp"
#include "ResourceMetaFile.hpp"

//-------------------------------------------------------------------Resource
ZilchDefineType(Resource, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}

void Resource::Initialize(const ResourceMetaFile& resourceMeta)
{
  mId = resourceMeta.mId;
  mName = resourceMeta.mName;
  mPath = resourceMeta.mResourcePath;
}

bool Resource::operator==(const Resource& rhs) const
{
  return mId == rhs.mId;
}

size_t Resource::Hash() const
{
  return mId.Hash();
}
