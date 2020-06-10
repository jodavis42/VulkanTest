#include "Precompiled.hpp"

#include "ZilchScriptZilchStaticLibrary.hpp"

#include "Engine/EngineZilchStaticLibrary.hpp"
#include "ZilchComponent.hpp"
#include "ZilchScriptManager.hpp"
#include "ZilchScriptExtensions.hpp"

ZilchDefineStaticLibrary(ZilchScriptStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(ZilchScript);

  ZilchInitializeType(ZilchComponent);
  ZilchInitializeTypeAs(ZilchScriptExtensions, "Script");

  AddNativeLibraryExtensions(builder);
}
