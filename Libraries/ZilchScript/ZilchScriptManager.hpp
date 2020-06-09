#pragma once

#include "EngineStandard.hpp"

#include "ResourceManager.hpp"

class ResourceMetaFile;

//-------------------------------------------------------------------ZilchScript
struct ZilchScript : public Resource
{
public:
  String mScriptContents;
};

//-------------------------------------------------------------------ZilchScriptManager
struct ZilchScriptManager : public ResourceManagerTyped<ZilchScript>
{
public:
  ZilchScriptManager();
  ~ZilchScriptManager();

  virtual void GetExtensions(Array<ResourceExtension>& extensions) const override;
  virtual bool OnLoadResource(const ResourceMetaFile& resourceMeta, ZilchScript* zilchScript) override;
  virtual bool OnReLoadResource(const ResourceMetaFile& resourceMeta, ZilchScript* zilchScript) override;

  Array<ZilchScript*> mModifiedScripts;
};
