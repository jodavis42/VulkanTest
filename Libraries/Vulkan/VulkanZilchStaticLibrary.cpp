#include "Precompiled.hpp"

#include "VulkanZilchStaticLibrary.hpp"

#include "Engine/EngineZilchStaticLibrary.hpp"

#include "SimpleRendererComponent.hpp"

ZilchDefineStaticLibrary(VulkanStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(SimpleRendererComponent);

  AddNativeLibraryExtensions(builder);
}

