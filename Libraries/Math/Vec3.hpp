#pragma once

#include <cmath>

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

  Vec3 operator-(const Vec3& rhs) const
  {
    Vec3 result = *this;
    result.x -= rhs.x;
    result.y -= rhs.y;
    result.z -= rhs.z;
    return result;
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

  static float Dot(const Vec3& lhs, const Vec3& rhs)
  {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
  }

  static Vec3 Cross(const Vec3& lhs, const Vec3& rhs)
  {
    Vec3 result;
    result.x = lhs.y * rhs.z - lhs.z * rhs.y;
    result.y = -(lhs.x * rhs.z - lhs.z * rhs.x);
    result.z = lhs.x * rhs.y - lhs.y * rhs.x;
    return result;
  }

  static Vec3 Normalized(const Vec3& vec)
  {
    Vec3 result = vec;
    float lengthSq = Vec3::Dot(vec, vec);
    float length = std::sqrt(lengthSq);
    result.x /= length;
    result.y /= length;
    result.z /= length;
    return result;
  }

  float x, y, z;
};
