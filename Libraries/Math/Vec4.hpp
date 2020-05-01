#pragma once

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
