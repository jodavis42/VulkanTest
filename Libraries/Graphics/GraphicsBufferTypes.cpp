#include "Precompiled.hpp"

#include "GraphicsBufferTypes.hpp"

namespace Zilch
{

ZilchDeclareExternalType(FrameData);
ZilchDefineExternalBaseType(FrameData, Zilch::TypeCopyMode::ReferenceType, builder, type)
{
  ZilchBindMember(mLogicTime);
  ZilchBindMember(mFrameTime);

  Zilch::HandleOf<BufferDescription> handle = ZilchAllocate(BufferDescription);
  handle->mBindingId = 0;
  handle->mBufferBindingType = ShaderMaterialBindingId::Global;
  handle->mDescriptorType = MaterialDescriptorType::Uniform;
  handle->mSizeInBytes = sizeof(FrameData);
  handle->mOffsetInBytes = 0;
  type->Add(*handle);
}

ZilchDeclareExternalType(CameraData);
ZilchDefineExternalBaseType(CameraData, Zilch::TypeCopyMode::ReferenceType, builder, type)
{
  ZilchBindMember(mNearPlane);
  ZilchBindMember(mFarPlane);
  ZilchBindMember(mViewportSize);

  Zilch::HandleOf<BufferDescription> handle = ZilchAllocate(BufferDescription);
  handle->mBindingId = 1;
  handle->mBufferBindingType = ShaderMaterialBindingId::Global;
  handle->mDescriptorType = MaterialDescriptorType::Uniform;
  handle->mSizeInBytes = sizeof(CameraData);
  handle->mOffsetInBytes = 0;
  type->Add(*handle);
}

ZilchDeclareExternalType(TransformData);
ZilchDefineExternalBaseType(TransformData, Zilch::TypeCopyMode::ReferenceType, builder, type)
{
  ZilchBindMember(mLocalToWorld);
  ZilchBindMember(mWorldToView);
  ZilchBindMember(mViewToPerspective);
  ZilchBindMember(mPerspectiveToApiPerspective);

  Zilch::HandleOf<BufferDescription> handle = ZilchAllocate(BufferDescription);
  handle->mBindingId = 2;
  handle->mBufferBindingType = ShaderMaterialBindingId::Transforms;
  handle->mDescriptorType = MaterialDescriptorType::UniformDynamic;
  handle->mSizeInBytes = sizeof(TransformData);
  handle->mOffsetInBytes = 0;
  type->Add(*handle);
}

ZilchDefineStaticLibrary(ZilchGraphicsLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(BufferDescription);
  ZilchInitializeExternalType(FrameData);
  ZilchInitializeExternalType(CameraData);
  ZilchInitializeExternalType(TransformData);
}

}//namespace Zilch

ZilchDefineType(BufferDescription, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}
