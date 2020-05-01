#pragma once

#include <stdint.h>
#include <utility>

#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Vec4.hpp"
#include "Integer2.hpp"
#include "Matrix3.hpp"
#include "Matrix4.hpp"

constexpr float cPi = 3.1415926535897932384626433832795f;

inline float ToRadians(float degrees)
{
  return (degrees * cPi) / 180;
}


namespace std
{

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

}//namespace std