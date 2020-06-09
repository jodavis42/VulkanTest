#include "Precompiled.hpp"

#include "Camera.hpp"

#include "Engine/Space.hpp"
#include "Engine/Transform.hpp"

#include "GraphicsSpace.hpp"
#include "RenderQueue.hpp"
#include "Renderer.hpp"

//-----------------------------------------------------------------------------Camera
ZilchDefineType(Camera, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindFieldProperty(mNearPlane);
  ZilchBindFieldProperty(mFarPlane);
  ZilchBindFieldProperty(mFov);
}

Camera::Camera()
{
}

void Camera::Initialize(const CompositionInitializer& initializer)
{
  Space* space = GetSpace();
  GraphicsSpace* graphicsSpace = space->Has<GraphicsSpace>();
  graphicsSpace->Add(this);
}

void Camera::OnDestroy()
{
  GraphicsSpace* graphicsSpace = GetSpace()->Has<GraphicsSpace>();
  graphicsSpace->Remove(this);
}

void Camera::FilloutViewBlock(const Renderer* renderer, ViewBlock& viewBlock) const
{
  size_t width, height;
  float aspectRatio;
  renderer->GetShape(width, height, aspectRatio);

  viewBlock.mWorldToView = GenerateWorldToViewMatrix();
  viewBlock.mViewToPerspective = renderer->BuildPerspectiveMatrix(Math::DegToRad(45.0f), aspectRatio, mNearPlane, mFarPlane);
  viewBlock.mViewToPerspective.Transpose();
  viewBlock.mNearPlane = mNearPlane;
  viewBlock.mFarPlane = mFarPlane;
  viewBlock.mViewportSize = Vec2(1);
}

Matrix4 Camera::GenerateWorldToViewMatrix() const
{
  Transform* transform = GetOwner()->Has <Transform>();

  Matrix4 rotation = Math::ToMatrix4(transform->mRotation);

  Matrix4 translation;
  translation.Translate(-transform->mTranslation);

  Matrix4 worldToView = rotation.Transposed() * translation;
  return worldToView;
}
