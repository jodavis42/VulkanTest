#pragma once

#include "GraphicsStandard.hpp"

struct Graphical;

struct GraphicalEntry
{
  Graphical* mGraphical = nullptr;
  uint64_t mSortId = 0;
  bool operator<(const GraphicalEntry& rhs) const
  {
    return mSortId < rhs.mSortId;
  }
};

