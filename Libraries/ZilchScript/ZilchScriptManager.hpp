#pragma once

#include "EngineStandard.hpp"

#include "ResourceManager.hpp"

class ResourceMetaFile;

//-------------------------------------------------------------------ZilchScript
struct ZilchScript : public Resource
{
public:
  ZilchDeclareType(ZilchScript, Zilch::TypeCopyMode::ReferenceType);
  String mScriptContents;
};

//-------------------------------------------------------------------ZilchScriptManager
struct ZilchScriptManager : public ResourceManagerTyped<ZilchScript>
{
public:
  ZilchDeclareType(ZilchScriptManager, Zilch::TypeCopyMode::ReferenceType);

  ZilchScriptManager();
  ~ZilchScriptManager();

  virtual void GetExtensions(Array<ResourceExtension>& extensions) const override;
  virtual bool OnLoadResource(const ResourceMetaFile& resourceMeta, ZilchScript* zilchScript) override;
  virtual bool OnReLoadResource(const ResourceMetaFile& resourceMeta, ZilchScript* zilchScript) override;

  Array<ZilchScript*> mModifiedScripts;
};
