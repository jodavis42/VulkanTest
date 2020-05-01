#pragma once

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
