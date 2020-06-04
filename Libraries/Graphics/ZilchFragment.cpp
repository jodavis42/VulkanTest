#include "Precompiled.hpp"

#include "ZilchFragment.hpp"

#include "ResourceMetaFile.hpp"

//-------------------------------------------------------------------ZilchFragmentFileManager
ZilchFragmentFileManager::ZilchFragmentFileManager()
{
 
}

ZilchFragmentFileManager::~ZilchFragmentFileManager()
{
}

void ZilchFragmentFileManager::GetExtensions(Array<ResourceExtension>& extensions) const
{
  extensions.PushBack({"zilchFrag"});
}

bool ZilchFragmentFileManager::OnLoadResource(const ResourceMetaFile& resourceMeta, ZilchFragmentFile* fragment)
{
  fragment->mFileContents = Zero::ReadFileIntoString(fragment->mPath);
  return true;
}

bool ZilchFragmentFileManager::OnReLoadResource(const ResourceMetaFile& resourceMeta, ZilchFragmentFile* fragment)
{
  fragment->mFileContents = Zero::ReadFileIntoString(fragment->mPath);
  return false;
}
