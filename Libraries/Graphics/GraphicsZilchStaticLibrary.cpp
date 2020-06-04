#include "Precompiled.hpp"

#include "GraphicsZilchStaticLibrary.hpp"
#include "GraphicsEngine.hpp"

ZilchDefineStaticLibrary(GraphicsStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(GraphicsEngine);
}
