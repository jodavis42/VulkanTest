#pragma once

#include <vector>
#include <unordered_map>
#include "GraphicsStandard.hpp"

//-------------------------------------------------------------------Vertex
struct Vertex
{
  Vec3 pos;
  Vec3 color;
  Vec2 uv;

  bool operator==(const Vertex& other) const
  {
    return pos == other.pos && color == other.color && uv == other.uv;
  }
};

namespace std
{

inline void hash_combine(size_t& seed, size_t hash)
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

}//namespace std
