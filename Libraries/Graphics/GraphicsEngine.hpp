#pragma once

#include "Mesh.hpp"
#include "Model.hpp"
#include "Texture.hpp"
#include "ZilchFragment.hpp"
#include "ZilchMaterial.hpp"
#include "ZilchShader.hpp"
#include "Renderer.hpp"
#include "VulkanRenderer.hpp"
#include <functional>

class GraphicsSpace;
class ResourceSystem;

struct GraphicsEngineRendererInitData
{
  SurfaceCreationDelegate mSurfaceCreationCallback;
  size_t mInitialWidth = 0;
  size_t mInitialHeight = 0;
};

struct GraphicsEngineInitData
{
  String mResourcesDir;
  String mShaderCoreDir;
  ResourceSystem* mResourceSystem = nullptr;
};

struct GraphicsEngine : public Zilch::EventHandler
{
public:
  void Initialize(const GraphicsEngineInitData& initData);
  void Shutdown();

  GraphicsSpace* CreateSpace(const String& name);
  GraphicsSpace* FindSpace(const String& name);
  void DestroySpace(const String& name);
  void DestroySpace(GraphicsSpace* space);

  void Update();
  Renderer* GetRenderer();

  void InitializeRenderer(GraphicsEngineRendererInitData& rendererInitData);
  void UploadImages();
  void UploadShaders();
  void UploadMaterial(ZilchMaterial* zilchMaterial);
  void UploadMaterials();
  void UploadMeshes();
  void ReloadResources();

  void OnResourceLoaded(ResourceLoadEvent* event);
  void OnResourceReLoaded(ResourceLoadEvent* event);

  void PopulateMaterialBuffer();
  void CreateSwapChain();
  void CleanupSwapChain();
  void RecreateSwapChain();
  void WaitIdle();

  SurfaceCreationDelegate mSurfaceCreationCallback;
  std::function<void(size_t&, size_t&)> mWindowSizeQueryFn = nullptr;
  Array<GraphicsSpace*> mSpaces;

  GraphicsEngineInitData mInitData;
  ResourceSystem* mResourceSystem = nullptr;
  MeshManager* mMeshManager = nullptr;
  TextureManager* mTextureManager = nullptr;
  ZilchFragmentFileManager* mZilchFragmentFileManager = nullptr;
  ZilchMaterialManager* mZilchMaterialManager = nullptr;
  ZilchShaderManager mZilchShaderManager;
  VulkanRenderer mRenderer;
  bool mReloadResources = false;
};
