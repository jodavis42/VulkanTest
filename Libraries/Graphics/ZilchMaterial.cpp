#include "Precompiled.hpp"

#include "ZilchMaterial.hpp"

#include "JsonSerializers.hpp"

//-------------------------------------------------------------------ZilchMaterialManager
ZilchMaterialManager::ZilchMaterialManager()
{

}

ZilchMaterialManager::~ZilchMaterialManager()
{
  Destroy();
}

void ZilchMaterialManager::Load(const String& resourcesDir)
{
  FileSearchData searchData = {resourcesDir, Zero::FilePath::Combine(resourcesDir, "data")};
  LoadAllFilesOfExtension(*this, searchData, ".zilchMaterial");
}

void ZilchMaterialManager::LoadFromFile(const FileLoadData& loadData)
{
  JsonLoader loader;
  loader.LoadFromFile(loadData.mFilePath);

  ZilchMaterial* material = new ZilchMaterial();
  material->mMaterialName = LoadDefaultPrimitive(loader, "Name", String());
  
  LoadZilchFragments(loader, material);
  mMaterialMap.InsertOrError(material->mMaterialName, material);
}

void ZilchMaterialManager::LoadZilchFragments(JsonLoader& loader, ZilchMaterial* material)
{
  if(loader.BeginMember("Fragments"))
  {
    size_t fragmentCount = 0;
    loader.BeginMembers(fragmentCount);
    material->mFragments.Resize(fragmentCount);
    for(size_t i = 0; i < fragmentCount; ++i)
    {
      MaterialFragment& fragment = material->mFragments[i];
      loader.BeginMember(i, fragment.mFragmentName);
      LoadZilchFragmentProperties(loader, fragment);

      loader.EndMember();
    }
    loader.EndMember();
  }
}

void ZilchMaterialManager::LoadZilchFragmentProperties(JsonLoader& loader, MaterialFragment& fragment)
{
  LoadMaterialProperties(loader, fragment.mProperties);
}

void ZilchMaterialManager::Add(const String& name, ZilchMaterial* material)
{
  mMaterialMap[name] = material;
}

ZilchMaterial* ZilchMaterialManager::Find(const String& name)
{
  return mMaterialMap.FindValue(name, nullptr);
}

void ZilchMaterialManager::Destroy()
{
  for(ZilchMaterial* material : mMaterialMap.Values())
    delete material;
  mMaterialMap.Clear();
}
