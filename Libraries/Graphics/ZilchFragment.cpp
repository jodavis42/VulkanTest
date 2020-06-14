#include "Precompiled.hpp"

#include "ZilchFragment.hpp"

#include "ResourceMetaFile.hpp"

//-------------------------------------------------------------------ZilchFragmentFile
ZilchDefineType(ZilchFragmentFile, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}

//-------------------------------------------------------------------ZilchFragmentFileManager
ZilchDefineType(ZilchFragmentFileManager, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();
}

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
  return true;
}
