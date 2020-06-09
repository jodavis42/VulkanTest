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
  zilchScript->mScriptContents = Zero::ReadFileIntoString(resourceMeta.mResourcePath);
  return true;
}

bool ZilchScriptManager::OnReLoadResource(const ResourceMetaFile& resourceMeta, ZilchScript* zilchScript)
{
  String newContents = Zero::ReadFileIntoString(resourceMeta.mResourcePath);
  if(newContents == zilchScript->mScriptContents)
    return false;

  zilchScript->mScriptContents = newContents;
  mModifiedScripts.PushBack(zilchScript);
  return true;
}
