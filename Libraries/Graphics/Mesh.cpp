#include "Precompiled.hpp"

#include "Mesh.hpp"

#include <filesystem>
#include "JsonSerializers.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

void FilloutMesh(Mesh* mesh, std::vector<tinyobj::shape_t>& shapes, tinyobj::attrib_t& attrib)
{
  std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

  for(const auto& shape : shapes)
  {
    for(const auto& index : shape.mesh.indices)
    {
      Vertex vertex = {};
      vertex.pos =
      {
        attrib.vertices[3 * index.vertex_index + 0],
        attrib.vertices[3 * index.vertex_index + 1],
        attrib.vertices[3 * index.vertex_index + 2]
      };

      vertex.uv =
      {
         attrib.texcoords[2 * index.texcoord_index + 0],
         1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
      };

      vertex.color = {1.0f, 1.0f, 1.0f};

      if(uniqueVertices.count(vertex) == 0)
      {
        uniqueVertices[vertex] = static_cast<uint32_t>(mesh->mVertices.Size());
        mesh->mVertices.PushBack(vertex);
      }

      mesh->mIndices.PushBack(uniqueVertices[vertex]);
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

void MeshManager::Load()
{
  LoadAllFilesOfExtension(*this, "data", ".mesh");
}

void MeshManager::LoadFromFile(const String& path)
{
  JsonLoader loader;
  loader.LoadFromFile(path);

  String meshName;
  String meshPath;

  LoadPrimitive(loader, "Name", meshName);
  LoadPrimitive(loader, "MeshPath", meshPath);
  LoadMesh(meshName, meshPath);
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
