#include "Precompiled.hpp"

#include "GraphicsZilchStaticLibrary.hpp"

#include "Engine/EngineZilchStaticLibrary.hpp"

#include "Camera.hpp"
#include "Mesh.hpp"
#include "Model.hpp"
#include "GraphicsSpace.hpp"
#include "GraphicsEngine.hpp"
#include "NativeRenderer.hpp"
#include "RenderGroup.hpp"
#include "RenderTasks.hpp"
#include "Texture.hpp"
#include "ZilchMaterial.hpp"
#include "ZilchFragment.hpp"

ZilchDefineStaticLibrary(GraphicsStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(Mesh);
  ZilchInitializeType(MeshManager);
  ZilchInitializeType(ZilchMaterial);
  ZilchInitializeType(ZilchMaterialManager);
  ZilchInitializeType(ZilchFragmentFile);
  ZilchInitializeType(ZilchFragmentFileManager);
  ZilchInitializeType(Texture);
  ZilchInitializeType(TextureManager);
  ZilchInitializeType(RenderGroup);
  ZilchInitializeType(RenderGroupManager);

  ZilchInitializeType(RenderGroupSet);
  ZilchInitializeType(RenderTask);
  ZilchInitializeType(ClearTargetRenderTask);
  ZilchInitializeType(RenderGroupRenderTask);
  ZilchInitializeType(RenderTaskEvent);

  ZilchInitializeType(Graphical);
  ZilchInitializeType(Camera);
  ZilchInitializeType(Model);
  ZilchInitializeType(NativeRenderer);
  ZilchInitializeType(GraphicsSpace);
  ZilchInitializeType(GraphicsEngine);

  AddNativeLibraryExtensions(builder);
}
