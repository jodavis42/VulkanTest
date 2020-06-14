#pragma once

#include "EngineStandard.hpp"

//-------------------------------------------------------------------CompositionInitializer
/// Initializer context for cogs/compositons. Currently empty but setup to allow easily adding more.
class CompositionInitializer
{
public:
};

//-------------------------------------------------------------------CompositionCreationContext
class CompositionCreationContext 
{
  ZilchDeclareType(CompositionCreationContext, Zilch::TypeCopyMode::ReferenceType);

  Vec3 mScale = Vec3(1, 1, 1);
  Quaternion mRotation = Quaternion::cIdentity;
  Vec3 mTranslation = Vec3(0, 0, 0);
};
