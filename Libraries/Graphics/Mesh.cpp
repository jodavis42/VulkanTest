#include "Precompiled.hpp"

#include "Mesh.hpp"
#undef Error

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

//-----------------------------------------------------------------------------Mesh
ZilchDefineType(Mesh, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}

//-------------------------------------------------------------------MeshManager
ZilchDefineType(MeshManager, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();
}

MeshManager::MeshManager()
{

}

MeshManager::~MeshManager()
{

}

void MeshManager::GetExtensions(Array<ResourceExtension>& extensions) const
{
  extensions.PushBack({"mesh"});
}

bool MeshManager::OnLoadResource(const ResourceMetaFile& resourceMeta, Mesh* mesh)
{
  return LoadMesh(resourceMeta, mesh);
}

bool MeshManager::OnReLoadResource(const ResourceMetaFile& resourceMeta, Mesh* mesh)
{
  return LoadMesh(resourceMeta, mesh);
}

bool MeshManager::LoadMesh(const ResourceMetaFile& resourceMeta, Mesh* mesh)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, resourceMeta.mResourcePath.c_str()))
    return false;

  FilloutMesh(mesh, shapes, attrib);
  return true;
}
