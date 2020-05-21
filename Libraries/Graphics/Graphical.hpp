#pragma once

#include "GraphicsStandard.hpp"

struct GraphicalFrameData;
class GraphicsSpace;

struct Graphical
{
  virtual void FilloutFrameData(GraphicalFrameData& frameData) const abstract;
  GraphicsSpace* mSpace = nullptr;
};
