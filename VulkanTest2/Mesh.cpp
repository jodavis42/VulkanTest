#include "pch.h"

#include "Mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"
const std::string MODEL_PATH = "models/chalet.obj";
#include <vulkan/vulkan.h>

void FilloutMesh(Mesh* mesh, std::vector<tinyobj::shape_t>& shapes, tinyobj::attrib_t& attrib)
{
  std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

  for(const auto& shape : shapes)
  {
    for(const auto& index : shape.mesh.indices)\
    {
      Vertex vertex = {};
      vertex.pos = {
        attrib.vertices[3 * index.vertex_index + 0],
        attrib.vertices[3 * index.vertex_index + 1],
        attrib.vertices[3 * index.vertex_index + 2]
      };

      vertex.uv = {
         attrib.texcoords[2 * index.texcoord_index + 0],
  1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
      };

      vertex.color = {1.0f, 1.0f, 1.0f};

      if(uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(mesh->mVertices.size());
        mesh->mVertices.push_back(vertex);
      }

      mesh->mIndices.push_back(uniqueVertices[vertex]);
    }
  }
}

MeshManager::MeshManager()
{

}

MeshManager::~MeshManager()
{
  Destroy();
}

void MeshManager::Load()
{
  LoadMesh("Test", MODEL_PATH);
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

void MeshManager::Destroy()
{
  for(auto pair : mMeshMap)
  {
    delete pair.second;
  }
  mMeshMap.clear();
}
