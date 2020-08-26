#include "Precompiled.hpp"

#include "RenderTarget.hpp"

//-------------------------------------------------------------------RenderTarget
ZilchDefineType(RenderTarget, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  type->CreatableInScript = true;
  ZilchBindField(mSize);
  ZilchBindField(mFormat);
}

bool RenderTarget::IsDepthFormat() const
{
  return mFormat == TextureFormat::Depth16 ||
    mFormat == TextureFormat::Depth24 ||
    mFormat == TextureFormat::Depth32 ||
    mFormat == TextureFormat::Depth32f;
}

bool RenderTarget::IsDepthStencilFormat() const
{
  return mFormat == TextureFormat::Depth24Stencil8 ||
    mFormat == TextureFormat::Depth32fStencil8Pad24;
}
