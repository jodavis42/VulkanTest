#include "Precompiled.hpp"

#include "ResourceId.hpp"

const ResourceId ResourceId::cInvalid = ResourceId{(static_cast<u64>(-1))};

ResourceId::ResourceId() : Zero::Guid(ResourceId::cInvalid)
{

}

ResourceId::ResourceId(u64 guid) : Zero::Guid(guid)
{

}

ResourceId::ResourceId(int64 guid) : Zero::Guid(guid)
{

}

ResourceId::ResourceId(Zero::Guid guid) : Zero::Guid(guid)
{

}

ResourceId::operator int64() const
{
  u64 guid = static_cast<u64>(*this);
  return *reinterpret_cast<int64*>(&guid);
}

size_t ResourceId::Hash() const
{
  return Zero::Guid::Hash();
}
