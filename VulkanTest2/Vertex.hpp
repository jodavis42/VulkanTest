#pragma once

struct Vec2
{
  Vec2() {}
  Vec2(float x_, float y_)
  {
    x = x_;
    y = y_;
  }

  bool operator==(const Vec2& rhs) const
  {
    return x == rhs.x && y == rhs.y;
  }

  float x, y;
};

struct Vec3
{
  Vec3() {}
  Vec3(float x_, float y_, float z_)
  {
    x = x_;
    y = y_;
    z = z_;
  }

  bool operator==(const Vec3& rhs) const
  {
    return x == rhs.x && y == rhs.y && z == rhs.z;
  }

  float x, y, z;
};

struct Vertex
{
  Vec3 pos;
  Vec3 color;
  Vec2 uv;

  static std::vector<VkVertexInputBindingDescription> getBindingDescription()
  {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    bindingDescriptions.resize(1);

    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescriptions;
  }

  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
  {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    attributeDescriptions.resize(3);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, uv);

    return attributeDescriptions;
  }

  bool operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color && uv == other.uv;
  }
};

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {

inline void hash_combine(size_t &seed, size_t hash)
{
  hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
  seed ^= hash;
}


template<> struct hash<Vec2>
{
  size_t operator()(const Vec2& v) const
  {
    size_t seed = 0;
    hash<float> hasher;
    hash_combine(seed, hasher(v.x));
    hash_combine(seed, hasher(v.y));
    return seed;
  }
};

template<> struct hash<Vec3>
{
  size_t operator()(const Vec3& v) const
  {
    size_t seed = 0;
    hash<float> hasher;
    hash_combine(seed, hasher(v.x));
    hash_combine(seed, hasher(v.y));
    hash_combine(seed, hasher(v.z));
    return seed;
  }
};

template<> struct hash<Vertex>
{
  size_t operator()(const Vertex& vertex) const
  {
    return ((hash<Vec3>()(vertex.pos) ^
            (hash<Vec3>()(vertex.color) << 1)) >> 1) ^
            (hash<Vec2>()(vertex.uv) << 1);
  }
};

}
