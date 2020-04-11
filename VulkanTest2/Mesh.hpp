#pragma once

#include "Vertex.hpp"

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
