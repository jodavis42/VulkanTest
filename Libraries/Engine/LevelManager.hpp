#pragma once

#include "EngineStandard.hpp"

#include "ResourceManager.hpp"

class ResourceMetaFile;

//-------------------------------------------------------------------Level
struct Level : public Resource
{
public:
};

//-------------------------------------------------------------------LevelManager
struct LevelManager : public ResourceManagerTyped<Level>
{
public:
  LevelManager();
  ~LevelManager();

  virtual void GetExtensions(Array<ResourceExtension>& extensions) const override;
  virtual bool OnLoadResource(const ResourceMetaFile& resourceMeta, Level* level) override;
  virtual bool OnReLoadResource(const ResourceMetaFile& resourceMeta, Level* level) override;
};
