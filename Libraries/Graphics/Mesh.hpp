#pragma once

#include "Vertex.hpp"
#include "GraphicsStandard.hpp"

struct FileLoadData;

//-------------------------------------------------------------------Mesh
struct Mesh
{
  Array<Vertex> mVertices;
  Array<uint32_t> mIndices;
};

//-------------------------------------------------------------------MeshManager
struct MeshManager
{
public:
  MeshManager();
  ~MeshManager();

  void Load(const String& resourcesDir);
  void LoadFromFile(const FileLoadData& loadData);
  void LoadMesh(const String& name, const String& path);
  Mesh* Find(const String& name);
  void Destroy();

  HashMap<String, Mesh*> mMeshMap;
};
