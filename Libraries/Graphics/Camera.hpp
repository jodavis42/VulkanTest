#pragma once

#include "Math.hpp"
#include "GraphicsStandard.hpp"
#include "Graphical.hpp"

struct Renderer;
struct ViewBlock;

//-----------------------------------------------------------------------------Camera
struct Camera : public Component
{
  ZilchDeclareType(Camera, Zilch::TypeCopyMode::ReferenceType);

  Camera();

  void Initialize(const CompositionInitializer& initializer);
  void OnDestroy();

  void FilloutViewBlock(const Renderer* renderer, ViewBlock& viewBlock) const;

  Matrix4 GenerateWorldToViewMatrix() const;

  float mNearPlane = 0.1f;
  float mFarPlane = 10.0f;
  float mFov = 45;
};
