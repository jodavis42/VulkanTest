#include "Precompiled.hpp"

#include "ResourceZilchStaticLibrary.hpp"

#include "ResourceManager.hpp"

ZilchDefineStaticLibrary(ResourceStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(ResourceLoadEvent);
  ZilchInitializeType(ResourceManager);
}
