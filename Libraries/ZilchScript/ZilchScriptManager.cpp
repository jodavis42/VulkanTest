#include "Precompiled.hpp"

#include "ZilchScriptManager.hpp"

#include "ResourceMetaFile.hpp"

//-------------------------------------------------------------------ZilchScriptManager
ZilchScriptManager::ZilchScriptManager()
{
 
}

ZilchScriptManager::~ZilchScriptManager()
{
}

void ZilchScriptManager::GetExtensions(Array<ResourceExtension>& extensions) const
{
  extensions.PushBack({"zilchScript"});
}

bool ZilchScriptManager::OnLoadResource(const ResourceMetaFile& resourceMeta, ZilchScript* zilchScript)
{
  return true;
}

bool ZilchScriptManager::OnReLoadResource(const ResourceMetaFile& resourceMeta, ZilchScript* zilchScript)
{
  return false;
}
