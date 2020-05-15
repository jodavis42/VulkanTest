#include "Precompiled.hpp"

#include "ZilchFragment.hpp"

#include "File.hpp"
#include "JsonSerializers.hpp"

//-------------------------------------------------------------------ZilchFragmentFileManager
ZilchFragmentFileManager::ZilchFragmentFileManager()
{
 
}

ZilchFragmentFileManager::~ZilchFragmentFileManager()
{
  Destroy();
}

void ZilchFragmentFileManager::Load(const String& resourcesDir)
{
  FileSearchData searchData = {resourcesDir, Zero::FilePath::Combine(resourcesDir, "ZilchFragments")};
  LoadAllFilesOfExtension(*this, searchData, ".zilchFrag");
}

void ZilchFragmentFileManager::LoadFromFile(const FileLoadData& loadData)
{
  ZilchFragmentFile* fragment = new ZilchFragmentFile();
  fragment->mFilePath = loadData.mFilePath;
  fragment->mFileContents = Zero::ReadFileIntoString(loadData.mFilePath);
  mResourceMap[fragment->mFilePath] = fragment;
}

ZilchFragmentFile* ZilchFragmentFileManager::Find(const String& name)
{
  return mResourceMap.FindValue(name, nullptr);
}

void ZilchFragmentFileManager::Destroy()
{
  for(ZilchFragmentFile* zilchSahder : mResourceMap.Values())
    delete zilchSahder;
  mResourceMap.Clear();
}
