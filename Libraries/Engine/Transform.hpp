#pragma once

#include "EngineStandard.hpp"

#include "Component.hpp"

//-------------------------------------------------------------------Transform
struct Transform : public Component
{
public:
  ZilchDeclareType(Transform, Zilch::TypeCopyMode::ReferenceType);

  Vec3 TransformDirection(const Vec3& direction) const;
  Vec3 TransformDirectionInverse(const Vec3& direction) const;
  Vec3 TransformPoint(const Vec3& point) const;
  Vec3 TransformPointInverse(const Vec3& point) const;
  Vec4 Multiply(const Vec4& value) const;
  Vec4 MultiplyInverse(const Vec4& value) const;
  Matrix4 GetWorldMatrix() const;

  Vec3 mScale = Vec3(1, 1, 1);
  Quaternion mRotation = Quaternion::cIdentity;
  Vec3 mTranslation = Vec3(0, 0, 0);
};
