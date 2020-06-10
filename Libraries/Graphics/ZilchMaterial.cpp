#include "Precompiled.hpp"

#include "ZilchMaterial.hpp"

#include "JsonSerializers.hpp"

//-------------------------------------------------------------------ZilchMaterial
ZilchDefineType(ZilchMaterial, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}

//-------------------------------------------------------------------ZilchMaterialManager
ZilchMaterialManager::ZilchMaterialManager()
{

}

ZilchMaterialManager::~ZilchMaterialManager()
{
}

void ZilchMaterialManager::GetExtensions(Array<ResourceExtension>& extensions) const
{
  extensions.PushBack({"zilchMaterial"});
}

bool ZilchMaterialManager::OnLoadResource(const ResourceMetaFile& resourceMeta, ZilchMaterial* zilchMaterial)
{
  return LoadZilchMaterial(resourceMeta, zilchMaterial);
}

bool ZilchMaterialManager::OnReLoadResource(const ResourceMetaFile& resourceMeta, ZilchMaterial* zilchMaterial)
{
  return LoadZilchMaterial(resourceMeta, zilchMaterial);
}

bool ZilchMaterialManager::LoadZilchMaterial(const ResourceMetaFile& resourceMeta, ZilchMaterial* zilchMaterial)
{
  JsonLoader loader;
  bool loadedFile = loader.LoadFromFile(resourceMeta.mResourcePath);
  if(!loadedFile)
  {
    Warn("Failure to load ZilchMaterial '%s'", resourceMeta.mResourcePath.c_str());
    return false;
  }

  zilchMaterial->mMaterialName = LoadDefaultPrimitive(loader, "Name", String());

  return LoadZilchFragments(loader, zilchMaterial);
}

bool ZilchMaterialManager::LoadZilchFragments(JsonLoader& loader, ZilchMaterial* material)
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
  return true;
}

void ZilchMaterialManager::LoadZilchFragmentProperties(JsonLoader& loader, MaterialFragment& fragment)
{
  LoadMaterialProperties(loader, fragment.mProperties);
}
