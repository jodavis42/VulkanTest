#include "Precompiled.hpp"

#include "Transform.hpp"

//-----------------------------------------------------------------------------Transform
ZilchDefineType(Transform, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindField(mScale);
  ZilchBindField(mRotation);
  ZilchBindField(mTranslation);

  ZilchBindMethod(TransformDirection);
  ZilchBindMethod(TransformDirectionInverse);
  ZilchBindMethod(TransformPoint);
  ZilchBindMethod(TransformPointInverse);
  ZilchBindMethod(Multiply);
  ZilchBindMethod(MultiplyInverse);
}

Vec3 Transform::TransformDirection(const Vec3& direction) const
{
  Matrix4 localToWorld = GetWorldMatrix();
  return Math::MultiplyNormal(localToWorld, direction);
}

Vec3 Transform::TransformDirectionInverse(const Vec3& direction) const
{
  Matrix4 localToWorld = GetWorldMatrix();
  Matrix4 worldToLocal = localToWorld.Inverted();
  return Math::MultiplyNormal(worldToLocal, direction);
}

Vec3 Transform::TransformPoint(const Vec3& point) const
{
  Matrix4 localToWorld = GetWorldMatrix();
  return Math::MultiplyPoint(localToWorld, point);
}

Vec3 Transform::TransformPointInverse(const Vec3& point) const
{
  Matrix4 localToWorld = GetWorldMatrix();
  Matrix4 worldToLocal = localToWorld.Inverted();
  return Math::MultiplyPoint(worldToLocal, point);
}

Vec4 Transform::Multiply(const Vec4& value) const
{
  Matrix4 localToWorld = GetWorldMatrix();
  return Math::Multiply(localToWorld, value);
}

Vec4 Transform::MultiplyInverse(const Vec4& value) const
{
  Matrix4 localToWorld = GetWorldMatrix();
  Matrix4 worldToLocal = localToWorld.Inverted();
  return Math::Multiply(worldToLocal, value);
}

Matrix4 Transform::GetWorldMatrix() const
{
  return Matrix4::GenerateTransform(mTranslation, mRotation, mScale);
}
