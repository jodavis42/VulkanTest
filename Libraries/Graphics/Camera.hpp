#pragma once

#include "Math.hpp"
#include "GraphicsStandard.hpp"
#include "Graphical.hpp"

struct Renderer;
struct ViewBlock;

struct Camera
{
  Camera();
  void FilloutViewBlock(const Renderer* renderer, ViewBlock& viewBlock) const;

  Matrix4 GenerateViewMatrix() const;

  float mNearPlane = 0.1f;
  float mFarPlane = 10.0f;
  float mFov = 45;
  Vec3 mPosition;
  Vec3 mTarget;
  Vec3 mWorldUp;
};
