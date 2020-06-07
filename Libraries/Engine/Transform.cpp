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
}

Matrix4 Transform::GetWorldMatrix() const
{
  return Matrix4::GenerateTransform(mTranslation, mRotation, mScale);
}
