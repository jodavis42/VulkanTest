#pragma once

#include "Vertex.hpp"
#include <string>
#include <unordered_map>
using String = std::string;

//-------------------------------------------------------------------Mesh
struct Mesh
{
  std::vector<Vertex> mVertices;
  std::vector<uint32_t> mIndices;
};

//-------------------------------------------------------------------MeshManager
struct MeshManager
{
public:
  MeshManager();
  ~MeshManager();

  void Load();
  void LoadFromFile(const String& path);
  void LoadMesh(const String& name, const String& path);
  Mesh* Find(const String& name);
  void Destroy();

  std::unordered_map<String, Mesh*> mMeshMap;
};
