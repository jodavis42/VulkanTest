#include "Precompiled.hpp"

#include "ResourceLibrary.hpp"
#include "Zilch/Zilch.hpp"
#include "ResourceExtensionManager.hpp"
#include "JsonSerializers.hpp"

ResourceLibrary::ResourceIdRangeOfResourceType::ResourceIdRangeOfResourceType(const ResourceTypeName& resouceTypeName, RangeType range)
  : mResourceTypeName(resouceTypeName)
  , mRange(range)
{

}

bool ResourceLibrary::ResourceIdRangeOfResourceType::Empty()
{
  return mRange.Empty();
}

ResourceLibrary::ResourceIdRangeOfResourceType::FrontType ResourceLibrary::ResourceIdRangeOfResourceType::Front()
{
  return mRange.Front().first;
}

void ResourceLibrary::ResourceIdRangeOfResourceType::PopFront()
{
  mRange.PopFront();
  while(!mRange.Empty() && mRange.Front().second.mResourceTypeName != mResourceTypeName)
    mRange.PopFront();
}

ResourceLibrary::ResourceIdRangeOfResourceType::SelfType ResourceLibrary::ResourceIdRangeOfResourceType::end()
{
  return SelfType(mResourceTypeName, mRange.end());
}

bool ResourceLibrary::ResourceIdRangeOfResourceType::operator==(const SelfType& rhs) const
{
  return mRange == rhs.mRange && mResourceTypeName == rhs.mResourceTypeName;
}

void ResourceLibrary::Load(ResourceExtensionManager& extensionManager)
{
  Load(extensionManager, mLibraryPath, mRecursiveLoad);
}

void ResourceLibrary::Load(ResourceExtensionManager& extensionManager, const String& path, bool recursive)
{
  Zero::FileRange range(path);
  for(; !range.Empty(); range.PopFront())
  {
    Zero::FileEntry fileEntry = range.FrontEntry();
    String fullPath = fileEntry.GetFullPath();
    if(Zero::DirectoryExists(fullPath) && recursive)
    {
      Load(extensionManager, fullPath, recursive);
    }

    Zero::FilePathInfo pathInfo = Zero::FilePath::GetPathInfo(fullPath);

    ResourceExtension extension = {pathInfo.Extension};
    ResourceTypeName* resourceTypeName = extensionManager.Find(extension);
    if(resourceTypeName == nullptr)
      continue;

    ResourceMetaFile metaFile = LoadMetaFileForResource({fullPath}, *resourceTypeName);
    mExtensionsToMetaFilePaths[extension].PushBack(metaFile.mId);
    mResourceIdMetaMap[metaFile.mId] = metaFile;
    mResourcePathToIdMap[metaFile.mResourcePath] = metaFile.mId;
  }
}

ResourceMetaFile ResourceLibrary::LoadMetaFileForResource(const ResourcePath& path, const ResourceTypeName& resourceTypeName)
{
  String metaFilePath = Zero::FilePath::CombineWithExtension(String(), path, ".meta");

  ResourceMetaFile metaFile;
  metaFile.mResourcePath = path;
  metaFile.mResourceTypeName = resourceTypeName;
  metaFile.mName = {Zero::FilePath::GetFileNameWithoutExtension(path)};

  JsonLoader loader;
  if(loader.LoadFromFile(metaFilePath))
  {
    if(loader.BeginMember("Id"))
    {
      int64 guid;
      if(loader.SerializePrimitive(guid))
        metaFile.mId = ResourceId(guid);
      loader.EndMember();
    }
    if(loader.BeginMember("Name"))
    {
      loader.SerializePrimitive(metaFile.mName);
      loader.EndMember();
    }
  }
  
  if(metaFile.mId == ResourceId::cInvalid)
  {
    metaFile.mId = ResourceId{ Zero::GenerateUniqueId64() };
    JsonSaver saver;
    saver.BeginObject();
    if(saver.BeginMember("Id"))
    {
      int64 guid = metaFile.mId;
      saver.WritePrimitive(guid);
    }
    if(saver.BeginMember("Name"))
      saver.WritePrimitive(metaFile.mName);
    saver.EndObject();
    Zero::WriteToFile(metaFilePath, saver.ToString());
  }

  return metaFile;
}

ResourceLibrary::ResourceIdRangeOfResourceType ResourceLibrary::AllResourcesOfType(const ResourceTypeName& resourceTypeName)
{
  return ResourceIdRangeOfResourceType(resourceTypeName, mResourceIdMetaMap.All());
}
