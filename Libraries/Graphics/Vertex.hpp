#pragma once

#include <vector>
#include <unordered_map>
#include "GraphicsStandard.hpp"

//-------------------------------------------------------------------Vertex
struct Vertex
{
  Vec3 pos = Vec3::cZero;
  Vec3 normal = Vec3::cZero;
  Vec4 color = Vec4::cZero;
  Vec2 uv = Vec2::cZero;
  Vec4 aux0 = Vec4::cZero;
};
