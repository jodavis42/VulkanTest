#include "Precompiled.hpp"

#include "Graphical.hpp"

ZilchDefineType(Graphical, builder, type)
{
  ZilchBindDestructor();

  ZilchBindFieldProperty(mRenderGroupSet);
}
