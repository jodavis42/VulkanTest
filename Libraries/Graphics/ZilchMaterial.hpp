#pragma once

#include "GraphicsStandard.hpp"
#include "MaterialShared.hpp"

struct FileLoadData;

//-------------------------------------------------------------------MaterialFragment
struct MaterialFragment
{
  String mFragmentName;
  Array<MaterialProperty> mProperties;
};

//-------------------------------------------------------------------ZilchMaterial
struct ZilchMaterial
{
  String mMaterialName;

  Array<MaterialFragment> mFragments;
};

//-------------------------------------------------------------------ZilchMaterialManager
struct ZilchMaterialManager
{
public:
  ZilchMaterialManager();
  ~ZilchMaterialManager();

  void Load(const String& resourcesDir);
  void LoadFromFile(const FileLoadData& loadData);
  void LoadZilchFragments(JsonLoader& loader, ZilchMaterial* material);
  void LoadZilchFragmentProperties(JsonLoader& loader, MaterialFragment& fragment);

  void Add(const String& name, ZilchMaterial* material);
  ZilchMaterial* Find(const String& name);
  void Destroy();

  HashMap<String, ZilchMaterial*> mMaterialMap;
};
