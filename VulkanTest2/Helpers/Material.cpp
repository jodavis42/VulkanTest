#include "pch.h"

#include "Material.hpp"

MaterialManager::MaterialManager()
{

}

MaterialManager::~MaterialManager()
{
  Destroy();
}

void MaterialManager::Add(const String& name, Material* material)
{
  mMaterialMap[name] = material;
}

Material* MaterialManager::Find(const String& name)
{
  auto it = mMaterialMap.find(name);
  if(it == mMaterialMap.end())
    return nullptr;
  return it->second;
}

void MaterialManager::Destroy()
{
  for(auto pair : mMaterialMap)
    delete pair.second;
  
  mMaterialMap.clear();
}
