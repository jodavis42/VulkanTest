#pragma once

#include "Vertex.hpp"
#include <string>
using String = std::string;

struct Mesh
{
  std::vector<Vertex> mVertices;
  std::vector<uint32_t> mIndices;
};

struct VulkanMesh
{
  VkBuffer mVertexBuffer;
  VkDeviceMemory mVertexBufferMemory;

  VkBuffer mIndexBuffer;
  VkDeviceMemory mIndexBufferMemory;
  uint32_t mIndexCount;
};

struct MeshManager
{
public:
  MeshManager();
  ~MeshManager();

  void Load();
  void LoadMesh(const String& name, const String& path);
  void Destroy();

  std::unordered_map<String, Mesh*> mMeshMap;
};
