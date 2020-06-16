#pragma once

#include "GraphicsStandard.hpp"

#include "Engine/Component.hpp"
#include "RenderGroup.hpp"

struct GraphicalFrameData;
class GraphicsSpace;

struct Graphical : public Component
{
  ZilchDeclareType(Graphical, Zilch::TypeCopyMode::ReferenceType);

  virtual void FilloutFrameData(GraphicalFrameData& frameData) const abstract;

  RenderGroupSet mRenderGroupSet;
  GraphicsSpace* mSpace = nullptr;
};
