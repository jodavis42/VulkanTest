#pragma once

#include <vector>
#include "Math.hpp"

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
