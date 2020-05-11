#include "Precompiled.hpp"

#include "Mesh.hpp"
#undef Error

#include <filesystem>
#include "JsonSerializers.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

void FilloutMesh(Mesh* mesh, std::vector<tinyobj::shape_t>& shapes, tinyobj::attrib_t& attrib)
{
  for(const auto& shape : shapes)
  {
    for(const auto& index : shape.mesh.indices)
    {
      Vertex vertex = {};
      vertex.pos = Vec3(&attrib.vertices[3 * index.vertex_index]);
      vertex.normal = Vec3(&attrib.normals[3 * index.normal_index]);
      vertex.uv = Vec2(&attrib.texcoords[2 * index.texcoord_index]);
    
      uint32_t vertexIndex = static_cast<uint32_t>(mesh->mVertices.Size());
      mesh->mVertices.PushBack(vertex);
      mesh->mIndices.PushBack(vertexIndex);
    }
  }
}

//-------------------------------------------------------------------MeshManager
MeshManager::MeshManager()
{

}

MeshManager::~MeshManager()
{
  Destroy();
}

void MeshManager::Load(const String& resourcesDir)
{
  FileSearchData searchData = {resourcesDir, Zero::FilePath::Combine(resourcesDir, "data")};
  LoadAllFilesOfExtension(*this, searchData, ".mesh");
}

void MeshManager::LoadFromFile(const FileLoadData& loadData)
{
  JsonLoader loader;
  loader.LoadFromFile(loadData.mFilePath);

  String meshName;
  String meshPath;

  LoadPrimitive(loader, "Name", meshName);
  LoadPrimitive(loader, "MeshPath", meshPath);

  String fullMeshPath = Zero::FilePath::Combine(loadData.mRootResourcesDir, meshPath);
  LoadMesh(meshName, fullMeshPath);
}

void MeshManager::LoadMesh(const String& name, const String& path)
{
  Mesh* mesh = new Mesh();
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
    throw std::runtime_error(warn + err);

  FilloutMesh(mesh, shapes, attrib);
  mMeshMap[name] = mesh;
}

Mesh* MeshManager::Find(const String& name)
{
  return mMeshMap.FindValue(name, nullptr);
}

void MeshManager::Destroy()
{
  for(Mesh* mesh : mMeshMap.Values())
    delete mesh;
  mMeshMap.Clear();
}
