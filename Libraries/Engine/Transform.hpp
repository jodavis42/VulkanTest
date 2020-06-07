#pragma once

#include "EngineStandard.hpp"

#include "Component.hpp"

//-------------------------------------------------------------------Transform
struct Transform : public Component
{
public:
  ZilchDeclareType(Transform, Zilch::TypeCopyMode::ReferenceType);

  Matrix4 GetWorldMatrix() const;

  Vec3 mScale = Vec3(1, 1, 1);
  Quaternion mRotation = Quaternion::cIdentity;
  Vec3 mTranslation = Vec3(0, 0, 0);
};
