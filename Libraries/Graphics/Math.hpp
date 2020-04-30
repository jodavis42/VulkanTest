#pragma once

#include <stdint.h>
#include <utility>

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

struct Matrix3
{
  typedef float scalar;

  Matrix3()
  {
    SetIdentity();
  }

  void Load(scalar* data)
  {
    for(size_t i = 0; i < 9; ++i)
      array[i] = data[i];
  }

  const Vec3& operator[](uint32_t index) const
  {
    return ((Vec3*)this)[index];
  }
  Vec3& operator[](uint32_t index)
  {
    return ((Vec3*)this)[index];
  }

  scalar operator()(uint32_t r, uint32_t c) const
  {
    return array[c + r * 3];
  }

  scalar& operator()(uint32_t r, uint32_t c)
  {
    return array[c + r * 3];
  }

  /// Set all elements to 0.
  void ZeroOut()
  {
    for(size_t i = 0; i < 9; ++i)
      array[i] = 0.0f;
  }

  /// Set the matrix to the identity.
  void SetIdentity()
  {
    m00 = 1.0f; m01 = 0.0f; m02 = 0.0f;
    m10 = 0.0f; m11 = 1.0f; m12 = 0.0f;
    m20 = 0.0f; m21 = 0.0f; m22 = 1.0f;
  }

  /// Return the transpose of the given matrix.
  static Matrix3 Transposed(const Matrix3& mat)
  {
    Matrix3 result = mat;
    std::swap(result.m01, result.m10);
    std::swap(result.m02, result.m20);
    std::swap(result.m12, result.m21);
    return result;
  }

  union
  {
    struct
    {
      scalar m00, m01, m02,
        m10, m11, m12,
        m20, m21, m22,
        m30, m31, m32;
    };

    scalar array[9];
  };
};

struct Matrix4
{

  typedef float scalar;

  Matrix4()
  {
    SetIdentity();
  }

  void Load(scalar* data)
  {
    for(size_t i = 0; i < 16; ++i)
      array[i] = data[i];
  }

  const Vec4& operator[](uint32_t index) const
  {
    return ((Vec4*)this)[index];
  }
  Vec4& operator[](uint32_t index)
  {
    return ((Vec4*)this)[index];
  }

  scalar operator()(uint32_t r, uint32_t c) const
  {
    return array[c + r * 4];
  }

  scalar& operator()(uint32_t r, uint32_t c)
  {
    return array[c + r * 4];
  }

  /// Set all elements to 0.
  void ZeroOut()
  {
    for(size_t i = 0; i < 16; ++i)
      array[i] = 0.0f;
  }

  /// Set the matrix to the identity.
  void SetIdentity()
  {
    m00 = 1.0f; m01 = 0.0f; m02 = 0.0f; m03 = 0.0f;
    m10 = 0.0f; m11 = 1.0f; m12 = 0.0f; m13 = 0.0f;
    m20 = 0.0f; m21 = 0.0f; m22 = 1.0f; m23 = 0.0f;
    m30 = 0.0f; m31 = 0.0f; m32 = 0.0f; m33 = .0f;
  }

  /// Return the transpose of the given matrix.
  static Matrix4 Transposed(const Matrix4& mat)
  {
    Matrix4 result = mat;
    std::swap(result.m01, result.m10);
    std::swap(result.m02, result.m20);
    std::swap(result.m03, result.m30);
    std::swap(result.m12, result.m21);
    std::swap(result.m13, result.m31);
    std::swap(result.m23, result.m32);
    return result;
  }

  union
  {
    struct
    {
      scalar m00, m01, m02, m03,
             m10, m11, m12, m13,
             m20, m21, m22, m23,
             m30, m31, m32, m33;
    };

    scalar array[16];
  };
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