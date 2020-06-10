#pragma once

#include "GraphicsStandard.hpp"
#include "MaterialShared.hpp"
#include "ResourceManager.hpp"

//-------------------------------------------------------------------MaterialFragment
struct MaterialFragment
{
  String mFragmentName;
  Array<MaterialProperty> mProperties;
};

//-------------------------------------------------------------------ZilchMaterial
struct ZilchMaterial : public Resource
{
  ZilchDeclareType(ZilchMaterial, Zilch::TypeCopyMode::ReferenceType);
  String mMaterialName;

  Array<MaterialFragment> mFragments;
};

//-------------------------------------------------------------------ZilchMaterialManager
struct ZilchMaterialManager : public ResourceManagerTyped<ZilchMaterial>
{
public:
  ZilchMaterialManager();
  ~ZilchMaterialManager();

  virtual void GetExtensions(Array<ResourceExtension>& extensions) const override;
  virtual bool OnLoadResource(const ResourceMetaFile& resourceMeta, ZilchMaterial* zilchMaterial) override;
  virtual bool OnReLoadResource(const ResourceMetaFile& resourceMeta, ZilchMaterial* zilchMaterial) override;
  
  bool LoadZilchMaterial(const ResourceMetaFile& resourceMeta, ZilchMaterial* zilchMaterial);
  bool LoadZilchFragments(JsonLoader& loader, ZilchMaterial* material);
  void LoadZilchFragmentProperties(JsonLoader& loader, MaterialFragment& fragment);
};
