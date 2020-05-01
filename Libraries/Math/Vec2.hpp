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
