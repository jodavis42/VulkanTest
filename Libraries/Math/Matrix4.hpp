#pragma once

#include "Vec3.hpp"
#include "Matrix3.hpp"

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
    m30 = 0.0f; m31 = 0.0f; m32 = 0.0f; m33 = 1.0f;
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

  static Matrix4 BuildTransform(const Vec3& scale, const Matrix3& rotation, const Vec3& translation)
  {
    Matrix4 result;
    //Translation component
    result.m03 = translation.x;
    result.m13 = translation.y;
    result.m23 = translation.z;
    result.m33 = scalar(1.0f);

    //Rotational component
    result.m00 = rotation.m00;
    result.m01 = rotation.m01;
    result.m02 = rotation.m02;

    result.m10 = rotation.m10;
    result.m11 = rotation.m11;
    result.m12 = rotation.m12;

    result.m20 = rotation.m20;
    result.m21 = rotation.m21;
    result.m22 = rotation.m22;

    //Scale component
    result.m00 *= scale.x;
    result.m10 *= scale.x;
    result.m20 *= scale.x;

    result.m01 *= scale.y;
    result.m11 *= scale.y;
    result.m21 *= scale.y;

    result.m02 *= scale.z;
    result.m12 *= scale.z;
    result.m22 *= scale.z;

    result.m30 = result.m31 = result.m32 = scalar(0.0f);
    return result;
  }

  static Matrix4 GenerateLookAt(const Vec3& eye, const Vec3& center, const Vec3& worldUp)
  {
    Vec3 forward = Vec3::Normalized(center - eye);
    Vec3 right = Vec3::Normalized(Vec3::Cross(forward, worldUp));
    Vec3 actualUp = Vec3::Cross(right, forward);

    Matrix4 result;
    result.SetIdentity();
    result.m00 = right.x;
    result.m10 = right.y;
    result.m20 = right.z;
    result.m01 = actualUp.x;
    result.m11 = actualUp.y;
    result.m21 = actualUp.z;
    result.m02 = -forward.x;
    result.m12 = -forward.y;
    result.m22 = -forward.z;
    result.m30 = -Vec3::Dot(right, eye);
    result.m31 = -Vec3::Dot(actualUp, eye);
    result.m32 = Vec3::Dot(forward, eye);
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
