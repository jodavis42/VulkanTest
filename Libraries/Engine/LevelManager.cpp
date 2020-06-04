#include "Precompiled.hpp"

#include "LevelManager.hpp"

#include "ResourceMetaFile.hpp"

//-------------------------------------------------------------------LevelManager
LevelManager::LevelManager()
{
 
}

LevelManager::~LevelManager()
{
}

void LevelManager::GetExtensions(Array<ResourceExtension>& extensions) const
{
  extensions.PushBack({"level"});
}

bool LevelManager::OnLoadResource(const ResourceMetaFile& resourceMeta, Level* level)
{
  return true;
}

bool LevelManager::OnReLoadResource(const ResourceMetaFile& resourceMeta, Level* level)
{
  return false;
}
