#pragma once

#include "GraphicsStandard.hpp"
#include "Zilch/Zilch.hpp"
#include "RenderPipelineEnums.hpp"
#include "TextureFormat.hpp"

//-------------------------------------------------------------------RenderTarget
struct RenderTarget
{
  ZilchDeclareType(RenderTarget, Zilch::TypeCopyMode::ReferenceType);

  using TargetId = uint32_t;
  static constexpr TargetId mInvalidTarget = static_cast<TargetId>(-1);
  static constexpr TargetId mFinalTargetId = static_cast<TargetId>(-2);

  bool IsDepthFormat() const;
  bool IsDepthStencilFormat() const;

  TextureFormat::Enum mFormat = TextureFormat::RGBA8;
  Zilch::Integer2 mSize = Zilch::Integer2::cZero;
  TargetId mId = mInvalidTarget;
};
