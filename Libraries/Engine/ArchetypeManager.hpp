#pragma once

#include "EngineStandard.hpp"

#include "ResourceManager.hpp"

class ResourceMetaFile;

//-------------------------------------------------------------------Archetype
struct Archetype : public Resource
{
public:
};

//-------------------------------------------------------------------ArchetypeManager
struct ArchetypeManager : public ResourceManagerTyped<Archetype>
{
public:
  ArchetypeManager();
  ~ArchetypeManager();

  virtual void GetExtensions(Array<ResourceExtension>& extensions) const override;
  virtual bool OnLoadResource(const ResourceMetaFile& resourceMeta, Archetype* archetype) override;
  virtual bool OnReLoadResource(const ResourceMetaFile& resourceMeta, Archetype* archetype) override;
};
