#pragma once

#include "GraphicsStandard.hpp"

struct Graphical;

struct GraphicalEntry
{
  Graphical* mGraphical = nullptr;
  uint64_t mSortId = 0;
};

