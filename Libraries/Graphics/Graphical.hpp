#pragma once

#include "GraphicsStandard.hpp"

#include "Engine/Component.hpp"

struct GraphicalFrameData;
class GraphicsSpace;

struct Graphical : public Component
{
  ZilchDeclareType(Graphical, Zilch::TypeCopyMode::ReferenceType);

  virtual void FilloutFrameData(GraphicalFrameData& frameData) const abstract;
  GraphicsSpace* mSpace = nullptr;
};
