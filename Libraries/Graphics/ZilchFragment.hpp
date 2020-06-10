#pragma once

#include "GraphicsStandard.hpp"

#include "ShaderEnumTypes.hpp"
#include "ResourceManager.hpp"

class ResourceMetaFile;

//-------------------------------------------------------------------ZilchFragmentFile
struct ZilchFragmentFile : public Resource
{
public:
  ZilchDeclareType(ZilchFragmentFile, Zilch::TypeCopyMode::ReferenceType);
  String mFileContents;
};

//-------------------------------------------------------------------ZilchFragmentFileManager
struct ZilchFragmentFileManager : public ResourceManagerTyped<ZilchFragmentFile>
{
public:
  ZilchFragmentFileManager();
  ~ZilchFragmentFileManager();

  virtual void GetExtensions(Array<ResourceExtension>& extensions) const override;
  virtual bool OnLoadResource(const ResourceMetaFile& resourceMeta, ZilchFragmentFile* fragment) override;
  virtual bool OnReLoadResource(const ResourceMetaFile& resourceMeta, ZilchFragmentFile* fragment) override;
};
