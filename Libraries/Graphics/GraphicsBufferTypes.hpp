#pragma once

#include "GraphicsStandard.hpp"
#include "MaterialShared.hpp"
#include "Zilch/Zilch.hpp"

namespace Zilch
{
ZilchDeclareStaticLibrary(ZilchGraphicsLibrary, ZilchNoNamespace, ZeroNoImportExport);
}//namespace Zilch

/// Metadata about a buffer used to describe how to map a description to a buffer.
struct BufferDescription
{
  ZilchDeclareType(BufferDescription, Zilch::TypeCopyMode::ReferenceType);

  u32 mBindingId = 0;
  MaterialDescriptorType mDescriptorType = MaterialDescriptorType::Unknown;
  ShaderMaterialBindingId::Enum mBufferBindingType = ShaderMaterialBindingId::Unknown;
  u32 mSizeInBytes = 0;
  u32 mOffsetInBytes = 0;
};

struct FrameData
{
  Zilch::Real mLogicTime;
  Zilch::Real mFrameTime;
};

struct CameraData
{
  Zilch::Real mNearPlane;
  Zilch::Real mFarPlane;
  Zilch::Real2 mViewportSize;
};

struct TransformData
{
  Zilch::Real4x4 mLocalToWorld;
  Zilch::Real4x4 mWorldToView;
  Zilch::Real4x4 mViewToPerspective;
  Zilch::Real4x4 mPerspectiveToApiPerspective;
};
