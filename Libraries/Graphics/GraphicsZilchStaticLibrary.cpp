#include "Precompiled.hpp"

#include "GraphicsZilchStaticLibrary.hpp"

#include "Engine/EngineZilchStaticLibrary.hpp"

#include "Camera.hpp"
#include "Mesh.hpp"
#include "Model.hpp"
#include "GraphicsSpace.hpp"
#include "GraphicsEngine.hpp"
#include "Texture.hpp"
#include "ZilchMaterial.hpp"
#include "ZilchFragment.hpp"

ZilchDefineStaticLibrary(GraphicsStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(Mesh);
  ZilchInitializeType(ZilchMaterial);
  ZilchInitializeType(ZilchFragmentFile);
  ZilchInitializeType(Texture);

  ZilchInitializeType(Graphical);
  ZilchInitializeType(Camera);
  ZilchInitializeType(Model);
  ZilchInitializeType(GraphicsSpace);
  ZilchInitializeType(GraphicsEngine);

  AddNativeLibraryExtensions(builder);
}
