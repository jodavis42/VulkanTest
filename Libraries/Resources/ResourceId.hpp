#pragma once

#include "ResourcesStandard.hpp"

#define DeclareResourceStringHashFn()       \
size_t Hash() const                         \
{                                           \
  return Zero::HashPolicy<String>()(*this); \
}

class ResourceStringType : public String
{
public:
  size_t Hash() const
  {
    return Zero::HashPolicy<String>()(*this);
  }
};

class ResourceId : public Zero::Guid
{
public:
  ResourceId();
  ResourceId(u64 guid);
  ResourceId(int64 guid);
  ResourceId(Zero::Guid guid);

  operator int64() const;
  size_t Hash() const;

  static const ResourceId cInvalid;
};

class ResourceName : public ResourceStringType
{
public:
  DeclareResourceStringHashFn();
};

class ResourceExtension : public ResourceStringType
{
public:
  DeclareResourceStringHashFn();
};

class ResourcePath : public ResourceStringType
{
public:
  DeclareResourceStringHashFn();
};

class ResourceTypeName : public ResourceStringType
{
public:
  DeclareResourceStringHashFn();
};

class ResourceManagerTypeName : public ResourceStringType
{
public:
  DeclareResourceStringHashFn();
};
