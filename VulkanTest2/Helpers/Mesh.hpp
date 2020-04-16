#pragma once

#include "Vertex.hpp"
#include <string>
#include <unordered_map>
using String = std::string;

struct Mesh
{
  std::vector<Vertex> mVertices;
  std::vector<uint32_t> mIndices;
};

struct MeshManager
{
public:
  MeshManager();
  ~MeshManager();

  void Load();
  void LoadMesh(const String& name, const String& path);
  Mesh* Find(const String& name);
  void Destroy();

  std::unordered_map<String, Mesh*> mMeshMap;
};
