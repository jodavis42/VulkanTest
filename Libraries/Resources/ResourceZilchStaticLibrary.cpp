#include "Precompiled.hpp"

#include "ResourceZilchStaticLibrary.hpp"

#include "ResourceManager.hpp"
#include "ResourceSet.hpp"
#include "ResourceSystem.hpp"

ZilchDefineStaticLibrary(ResourceStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(ResourceLoadEvent);
  ZilchInitializeType(Resource);
  ZilchInitializeType(ResourceManager);
  ZilchInitializeType(ResourceSystem);
  ZilchInitializeType(ResourceSet);
}
