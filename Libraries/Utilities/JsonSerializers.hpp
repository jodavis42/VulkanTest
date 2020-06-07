#pragma once

#include <functional>
#include <filesystem>
#include "Common/CommonStandard.hpp"

using Zero::String;
using Zero::Array;

class JsonInternalData;

struct PolymorphicInfo
{
  String mName;
};

class JsonSaver
{
public:

  JsonSaver();
  ~JsonSaver();
  String ToString();

  virtual bool SerializePrimitive(char* data);
  virtual bool SerializePrimitive(bool& data);
  virtual bool SerializePrimitive(char& data);
  virtual bool SerializePrimitive(int& data);
  virtual bool SerializePrimitive(int64& data);
  virtual bool SerializePrimitive(float& data);
  virtual bool SerializePrimitive(double& data);
  virtual bool SerializePrimitive(String& data);

  virtual bool BeginObject();
  virtual bool BeginObject(PolymorphicInfo& info);
  virtual bool BeginMembers(size_t& count);
  virtual bool BeginMember(const String& name);
  virtual bool BeginMember(size_t index, String& name);
  virtual bool BeginArray(size_t& count);
  virtual bool BeginArrayItem(size_t index);
  virtual bool EndObject();
  virtual bool EndMember();
  virtual bool EndMembers();
  virtual bool EndArray();
  virtual bool EndArrayItem();

  bool WriteKey(const String& name);
  bool WritePrimitive(bool data);
  bool WritePrimitive(char data);
  bool WritePrimitive(int data);
  bool WritePrimitive(int64 data);
  bool WritePrimitive(float data);
  bool WritePrimitive(double data);
  bool WritePrimitive(const String& data);

  JsonInternalData* mData;
};

class JsonLoader
{
public:
  JsonLoader();
  ~JsonLoader();

  bool Load(const String& jsonData);
  bool LoadFromFile(const String& filePath);

  virtual bool SerializePrimitive(char* data);
  virtual bool SerializePrimitive(bool& data);
  virtual bool SerializePrimitive(char& data);
  virtual bool SerializePrimitive(int& data);
  virtual bool SerializePrimitive(int64& data);
  virtual bool SerializePrimitive(float& data);
  virtual bool SerializePrimitive(double& data);
  virtual bool SerializePrimitive(String& data);

  virtual bool BeginObject();
  virtual bool BeginObject(PolymorphicInfo& info);
  virtual bool BeginMembers(size_t& count);
  virtual bool BeginMember(const String& name);
  virtual bool BeginMember(size_t index, String& name);
  virtual bool BeginArray(size_t& count);
  virtual bool BeginArrayItem(size_t index);
  virtual bool EndObject();
  virtual bool EndMembers();
  virtual bool EndMember();
  virtual bool EndArray();
  virtual bool EndArrayItem();

  bool ReadPrimitive(bool& data);
  bool ReadPrimitive(char& data);
  bool ReadPrimitive(int& data);
  bool ReadPrimitive(int64& data);
  bool ReadPrimitive(float& data);
  bool ReadPrimitive(double& data);
  bool ReadPrimitive(String& data);

  bool BeginMember();
  bool End();

  JsonInternalData* mData;
};

template <typename ArrayType, uint ArraySize = ArrayType::Count>
bool LoadArray(JsonLoader& loader, ArrayType& data)
{
  size_t count;
  loader.BeginArray(count);
  for(uint i = 0; i < count; ++i)
  {
    loader.BeginArrayItem(i);
    loader.SerializePrimitive(data[i]);
    loader.EndArrayItem();
  }
  loader.EndArray();
  return true;
}

template <typename ArrayType, uint ArraySize = ArrayType::Count>
bool LoadArray(JsonLoader& loader, const String& name, ArrayType& data)
{
  if(!loader.BeginMember(name))
    return false;

  size_t count;
  loader.BeginArray(count);
  for(uint i = 0; i < count; ++i)
  {
    loader.BeginArrayItem(i);
    loader.SerializePrimitive(data[i]);
    loader.EndArrayItem();
  }
  loader.EndArray();
  loader.EndMember();
  return true;
}
template <typename DataType>
bool LoadPrimitive(JsonLoader& loader, const String& name, DataType& data)
{
  if(!loader.BeginMember(name))
    return false;

  loader.SerializePrimitive(data);
  loader.EndMember();
  return true;
}
template <typename DataType>
DataType LoadDefaultPrimitive(JsonLoader& loader, const String& name, const DataType& defaultValue)
{
  DataType result = defaultValue;
  LoadPrimitive(loader, name, result);
  return result;
}

struct FileSearchData
{
  String mRootResourcesDir;
  String mSearchDir;
};

struct FileLoadData
{
  String mRootResourcesDir;
  String mFileDirectory;
  String mFilePath;
};

template <typename ManagerType>
void LoadAllFilesOfExtension(ManagerType& manager, const FileSearchData& searchData, const String& extension, std::function<void (const FileLoadData&)> callback, bool recursive = true)
{
  for(auto it : std::filesystem::directory_iterator(searchData.mSearchDir.c_str()))
  {
    if(it.is_directory())
    {
      if(recursive)
      {
        FileSearchData subSearchData{searchData.mRootResourcesDir, String(it.path().string().c_str())};
        LoadAllFilesOfExtension(manager, subSearchData, extension, recursive);
      }
      continue;
    }

    auto filePath = it.path();
    String fileExt = filePath.extension().string().c_str();
    if(fileExt == extension)
    {
      FileLoadData loadData;
      loadData.mRootResourcesDir = searchData.mRootResourcesDir;
      loadData.mFileDirectory = filePath.parent_path().string().c_str();
      loadData.mFilePath = filePath.string().c_str();
      callback(loadData);
    }
  }
}

template <typename ManagerType>
void LoadAllFilesOfExtension(ManagerType& manager, const FileSearchData& searchData, const String& extension, bool recursive = true)
{
  auto callback = [&manager](const FileLoadData& loadData)
  {
    manager.LoadFromFile(loadData);
  };
  LoadAllFilesOfExtension(manager, searchData, extension, callback, recursive);
}
