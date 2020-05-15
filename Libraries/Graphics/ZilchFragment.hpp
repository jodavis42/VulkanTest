#pragma once

#include "GraphicsStandard.hpp"

#include "ShaderEnumTypes.hpp"

struct FileLoadData;

//-------------------------------------------------------------------ZilchFragmentFile
struct ZilchFragmentFile
{
public:
  String mFilePath;
  String mFileContents;
};

//-------------------------------------------------------------------ZilchFragmentFileManager
struct ZilchFragmentFileManager
{
public:
  ZilchFragmentFileManager();
  ~ZilchFragmentFileManager();

  void Load(const String& resourcesDir);
  void LoadFromFile(const FileLoadData& loadData);
  ZilchFragmentFile* Find(const String& name);
  void Destroy();

  HashMap<String, ZilchFragmentFile*> mResourceMap;
};
