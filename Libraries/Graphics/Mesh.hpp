#pragma once

#include "Vertex.hpp"
#include "GraphicsStandard.hpp"
#include "ResourceManager.hpp"

//-------------------------------------------------------------------Mesh
struct Mesh : public Resource
{
  ZilchDeclareType(Mesh, Zilch::TypeCopyMode::ReferenceType);

  Array<Vertex> mVertices;
  Array<uint32_t> mIndices;
};

//-------------------------------------------------------------------MeshManager
struct MeshManager : public ResourceManagerTyped<Mesh>
{
public:
  MeshManager();
  ~MeshManager();

  virtual void GetExtensions(Array<ResourceExtension>& extensions) const override;
  virtual bool OnLoadResource(const ResourceMetaFile& resourceMeta, Mesh* mesh) override;
  virtual bool OnReLoadResource(const ResourceMetaFile& resourceMeta, Mesh* mesh) override;

  bool LoadMesh(const ResourceMetaFile& resourceMeta, Mesh* mesh);
};
