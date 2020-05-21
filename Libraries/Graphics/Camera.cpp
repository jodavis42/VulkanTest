#include "Precompiled.hpp"

#include "Camera.hpp"

#include "RenderQueue.hpp"
#include "Renderer.hpp"

Camera::Camera()
{
  mPosition = Vec3(5.0f, 5.0f, 5.0f);
  mTarget = Vec3(0.0f, 0.0f, 0.0f);
  mWorldUp = Vec3(0.0f, 0.0f, 1.0f);
}

void Camera::FilloutViewBlock(const Renderer* renderer, ViewBlock& viewBlock) const
{
  size_t width, height;
  float aspectRatio;
  renderer->GetShape(width, height, aspectRatio);

  viewBlock.mWorldToView = GenerateViewMatrix();
  viewBlock.mViewToPerspective = renderer->BuildPerspectiveMatrix(Math::DegToRad(45.0f), aspectRatio, mNearPlane, mFarPlane);
  viewBlock.mViewToPerspective.Transpose();
  viewBlock.mNearPlane = mNearPlane;
  viewBlock.mFarPlane = mFarPlane;
  viewBlock.mViewportSize = Vec2(1);
}

Matrix4 Camera::GenerateViewMatrix() const
{
  Vec3 forward = Vec3::Normalized(mTarget - mPosition);
  Vec3 right = Vec3::Normalized(Vec3::Cross(forward, mWorldUp));
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
  result.m30 = -Vec3::Dot(right, mPosition);
  result.m31 = -Vec3::Dot(actualUp, mPosition);
  result.m32 = Vec3::Dot(forward, mPosition);
  result.Transpose();
  return result;
}
