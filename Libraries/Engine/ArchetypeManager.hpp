#pragma once

#include "EngineStandard.hpp"

#include "ResourceManager.hpp"

class ResourceMetaFile;

//-------------------------------------------------------------------Archetype
struct Archetype : public Resource
{
public:
  ZilchDeclareType(Archetype, Zilch::TypeCopyMode::ReferenceType);
};

//-------------------------------------------------------------------ArchetypeManager
struct ArchetypeManager : public ResourceManagerTyped<Archetype>
{
public:
  ZilchDeclareType(ArchetypeManager, Zilch::TypeCopyMode::ReferenceType);

  ArchetypeManager();
  ~ArchetypeManager();

  virtual void GetExtensions(Array<ResourceExtension>& extensions) const override;
  virtual bool OnLoadResource(const ResourceMetaFile& resourceMeta, Archetype* archetype) override;
  virtual bool OnReLoadResource(const ResourceMetaFile& resourceMeta, Archetype* archetype) override;
};
