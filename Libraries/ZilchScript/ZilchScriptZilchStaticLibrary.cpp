#include "Precompiled.hpp"

#include "ZilchScriptZilchStaticLibrary.hpp"

#include "Engine/EngineZilchStaticLibrary.hpp"
#include "ZilchComponent.hpp"
#include "ZilchScriptExtensions.hpp"

ZilchDefineStaticLibrary(ZilchScriptStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(ZilchComponent);
  ZilchInitializeTypeAs(ZilchScriptExtensions, "ZilchScript");

  AddNativeLibraryExtensions(builder);
}
