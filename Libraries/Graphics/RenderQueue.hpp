#pragma once

#include "GraphicsStandard.hpp"
#include "Zilch/Zilch.hpp"
#include "RenderTasks.hpp"

struct FrameBlock
{
  float mFrameTime = 0.0f;
  float mLogicTime = 0.0f;
};

struct ViewBlock
{
  uint32_t mFrameBlockId = static_cast<uint32_t>(-1);
  Zilch::Real mNearPlane;
  Zilch::Real mFarPlane;
  Zilch::Real2 mViewportSize;

  Zilch::Real4x4 mWorldToView;
  Zilch::Real4x4 mViewToPerspective;
  RenderTaskEvent mRenderTaskEvent;
};

struct RenderQueue
{
  Array<FrameBlock> mFrameBlocks;
  Array<ViewBlock> mViewBlocks;
};

