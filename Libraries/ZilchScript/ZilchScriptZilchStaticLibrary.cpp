#include "Precompiled.hpp"

#include "ZilchScriptZilchStaticLibrary.hpp"

#include "Engine/EngineZilchStaticLibrary.hpp"
#include "ZilchComponent.hpp"
#include "ZilchScriptManager.hpp"
#include "ZilchScriptLibrary.hpp"
#include "ZilchScriptExtensions.hpp"

ZilchDefineStaticLibrary(ZilchScriptStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(ZilchScript);
  ZilchInitializeType(ZilchScriptManager);
  ZilchInitializeType(ZilchScriptLibrary);

  ZilchInitializeType(ZilchComponent);
  ZilchInitializeTypeAs(ZilchScriptExtensions, "Script");

  AddNativeLibraryExtensions(builder);
}
