#pragma once

struct Vec2
{
  static constexpr size_t Count = 2;

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
  float& operator[](size_t index)
  {
    float* data = &x;
    return data[index];
  }

  float x, y;
};

struct Vec3
{
  static constexpr size_t Count = 3;

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
  float& operator[](size_t index)
  {
    float* data = &x;
    return data[index];
  }

  float x, y, z;
};

struct Vec4
{
  static constexpr size_t Count = 4;

  Vec4() {}
  Vec4(float x_, float y_, float z_, float w_)
  {
    x = x_;
    y = y_;
    z = z_;
    w = w_;
  }

  bool operator==(const Vec4& rhs) const
  {
    return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
  }
  float& operator[](size_t index)
  {
    float* data = &x;
    return data[index];
  }

  float x, y, z, w;

};
struct Integer2
{
  typedef signed int scalar;
  static constexpr size_t Count = 2;
  Integer2() {}
  Integer2(scalar x_, scalar y_)
  {
    x = x_;
    y = y_;
  }

  bool operator==(const Integer2& rhs) const
  {
    return x == rhs.x && y == rhs.y;
  }
  scalar& operator[](size_t index)
  {
    scalar* data = &x;
    return data[index];
  }

  scalar x, y;
};

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