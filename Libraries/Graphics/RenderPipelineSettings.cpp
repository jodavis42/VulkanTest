#include "Precompiled.hpp"

#include "RenderPipelineSettings.hpp"
#include "Utilities/Hasher.hpp"

//-------------------------------------------------------------------BlendSettings
ZilchDefineType(BlendSettings, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  type->CreatableInScript = true;
  ZilchBindField(mBlendMode);
  ZilchBindField(mColorSourceFactor);
  ZilchBindField(mColorDestFactor);
  ZilchBindField(mColorBlendEquation);
  ZilchBindField(mAlphaSourceFactor);
  ZilchBindField(mAlphaDestFactor);
  ZilchBindField(mAlphaBlendEquation);
}

size_t BlendSettings::Hash() const
{
  Hasher hasher;
  hasher.U32(mBlendMode);
  hasher.U32(mColorSourceFactor);
  hasher.U32(mColorDestFactor);
  hasher.U32(mColorBlendEquation);
  hasher.U32(mAlphaSourceFactor);
  hasher.U32(mAlphaDestFactor);
  hasher.U32(mAlphaBlendEquation);
  return hasher.mHash;
}

//-------------------------------------------------------------------DepthSettings
ZilchDefineType(DepthSettings, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  type->CreatableInScript = true;
  ZilchBindField(mDepthMode);
  ZilchBindField(mDepthCompareFunc);
  ZilchBindField(mDepthBoundsTestEnable);
  ZilchBindField(mMinDepthBounds);
  ZilchBindField(mMaxDepthBounds);
}

size_t DepthSettings::Hash() const
{
  Hasher hasher;
  hasher.U32(mDepthMode);
  hasher.U32(mDepthCompareFunc);
  hasher(mDepthBoundsTestEnable);
  hasher(mMinDepthBounds);
  hasher(mMaxDepthBounds);
  return hasher.mHash;
}

//-------------------------------------------------------------------RenderPipelineSettings
ZilchDefineType(RenderPipelineSettings, builder, type)
{
  ZilchBindDefaultCopyDestructor();
  type->CreatableInScript = true;
  ZilchBindMember(mBlendSettings);
  ZilchBindMember(mDepthSettings);
}

size_t RenderPipelineSettings::Hash() const
{
  Hasher hasher;
  hasher.U64(mBlendSettings.Hash());
  hasher.U64(mDepthSettings.Hash());
  return hasher.mHash;
}
