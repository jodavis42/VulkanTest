#include "Precompiled.hpp"

#include "GraphicsZilchStaticLibrary.hpp"
#include "Model.hpp"
#include "GraphicsSpace.hpp"
#include "GraphicsEngine.hpp"
#include "Engine/EngineZilchStaticLibrary.hpp"

ZilchDefineStaticLibrary(GraphicsStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(Graphical);
  ZilchInitializeType(Model);
  ZilchInitializeType(GraphicsSpace);
  ZilchInitializeType(GraphicsEngine);

  AddNativeLibraryExtensions(builder);
}
