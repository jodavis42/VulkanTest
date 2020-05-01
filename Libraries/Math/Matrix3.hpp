#pragma once

#include "Vec3.hpp"
#include <cmath>

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

  static Matrix3 BuildRotation(const Vec3& axis, scalar angle)
  {
    Matrix3 result;
    scalar c0 = std::cos(angle);
    scalar n1C0 = scalar(1.0f) - c0;
    scalar s0 = std::sin(angle);

    //| x^2(1-c0)+c0  xy(1-c0)-zs0  xz(1-c0)+ys0 |
    //| xy(1-c0)+zs0  y^2(1-c0)+c0  yz(1-c0)-xs0 |
    //| xz(1-c0)-ys0  yz(1-c0)+xs0  z^2(1-c0)+c0 |
    result.m00 = axis.x * axis.x * n1C0 + c0;
    result.m01 = axis.x * axis.y * n1C0 - axis.z * s0;
    result.m02 = axis.x * axis.z * n1C0 + axis.y * s0;

    result.m10 = axis.x * axis.y * n1C0 + axis.z * s0;
    result.m11 = axis.y * axis.y * n1C0 + c0;
    result.m12 = axis.y * axis.z * n1C0 - axis.x * s0;

    result.m20 = axis.x * axis.z * n1C0 - axis.y * s0;
    result.m21 = axis.y * axis.z * n1C0 + axis.x * s0;
    result.m22 = axis.z * axis.z * n1C0 + c0;
    return result;
  }

  union
  {
    struct
    {
      scalar m00, m01, m02,
        m10, m11, m12,
        m20, m21, m22;
    };

    scalar array[9];
  };
};
