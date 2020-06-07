#include "Precompiled.hpp"

#include "ZilchScriptZilchStaticLibrary.hpp"

#include "Engine/EngineZilchStaticLibrary.hpp"
#include "ZilchComponent.hpp"

ZilchDefineStaticLibrary(ZilchScriptStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(ZilchComponent);

  AddNativeLibraryExtensions(builder);
}
