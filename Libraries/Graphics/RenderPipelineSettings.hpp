#pragma once

#include "GraphicsStandard.hpp"
#include "Zilch/Zilch.hpp"
#include "RenderPipelineEnums.hpp"

using TargetId = uint32_t;
using ObjectId = size_t;
constexpr TargetId mInvalidTarget = static_cast<TargetId>(-1);

//-------------------------------------------------------------------BlendSettings
struct BlendSettings
{
  ZilchDeclareType(BlendSettings, Zilch::TypeCopyMode::ValueType);

  size_t Hash() const;
  bool operator==(const BlendSettings& rhs) const = default;

  BlendMode::Enum mBlendMode = BlendMode::Enabled;

  BlendFactor::Enum mColorSourceFactor = BlendFactor::Zero;
  BlendFactor::Enum mColorDestFactor = BlendFactor::One;
  BlendEquation::Enum mColorBlendEquation = BlendEquation::Add;

  BlendFactor::Enum mAlphaSourceFactor = BlendFactor::Zero;
  BlendFactor::Enum mAlphaDestFactor = BlendFactor::One;
  BlendEquation::Enum mAlphaBlendEquation = BlendEquation::Add;
};

//-------------------------------------------------------------------DepthSettings
struct DepthSettings
{
  ZilchDeclareType(DepthSettings, Zilch::TypeCopyMode::ValueType);

  size_t Hash() const;
  bool operator==(const DepthSettings& rhs) const = default;

  DepthMode::Enum mDepthMode = DepthMode::ReadWrite;
  TextureCompareFunc::Enum mDepthCompareFunc = TextureCompareFunc::Less;
  bool mDepthBoundsTestEnable = false;
  float mMinDepthBounds = 0.0f;
  float mMaxDepthBounds = 1.0f;
};

//-------------------------------------------------------------------RenderPipelineSettings
struct RenderPipelineSettings
{
  ZilchDeclareType(RenderPipelineSettings, Zilch::TypeCopyMode::ValueType);

  size_t Hash() const;
  bool operator==(const RenderPipelineSettings& rhs) const = default;

  TargetId mColorTargetId = mInvalidTarget;
  BlendSettings mBlendSettings;
  DepthSettings mDepthSettings;
};
