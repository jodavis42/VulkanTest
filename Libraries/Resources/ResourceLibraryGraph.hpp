#pragma once

#include "ResourcesStandard.hpp"

#include "Resource.hpp"
#include "ResourceMetaFile.hpp"

class ResourceLibrary;

//-------------------------------------------------------------------ResourceLibraryGraph
/// Stores the dependency graph of resource libraries (only currently a dependency stack).
/// This owns the resource library memory;
class ResourceLibraryGraph
{
public:
  struct Range
  {
    using RangeType = Array<ResourceLibrary*>::range;
    using FrontType = RangeType::reference;
    RangeType mRange;

    Range(RangeType range) : mRange(range) {}
    bool Empty() { return mRange.Empty(); }
    FrontType Front() { return mRange.Back(); }
    void PopFront() { mRange.PopBack(); }

    // C++ iterator/range interface
    Range begin() { return *this; }
    Range end() { return RangeType(mRange.Begin(), mRange.Begin()); }
    bool operator==(const Range& rhs) const { return mRange.mBegin == rhs.mRange.mBegin && mRange.mEnd == rhs.mRange.mEnd; }
    bool operator!=(const Range& rhs) const { return !(*this == rhs); }
    Range& operator++() { PopFront();  return *this; }
    FrontType operator*() { return Front(); }
  };

  ResourceLibraryGraph();
  ~ResourceLibraryGraph();

  void PushLibrary(ResourceLibrary* library);
  void PopLibrary();

  Range GetLibraries();

private:
  void DestroyLibrary(ResourceLibrary* library);

  Array<ResourceLibrary*> mLibraryStack;
};
