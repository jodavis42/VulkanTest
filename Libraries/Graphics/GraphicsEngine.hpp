#pragma once

#include "Mesh.hpp"
#include "Model.hpp"
#include "Texture.hpp"
#include "ZilchFragment.hpp"
#include "ZilchMaterial.hpp"
#include "ZilchShader.hpp"
#include "Renderer.hpp"
#include "VulkanRenderer.hpp"
#include "Engine/Component.hpp"
#include <functional>

class GraphicsSpace;
class ResourceSystem;
class UpdateEvent;

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

struct GraphicsEngine : public Component
{
public:
  ZilchDeclareType(GraphicsEngine, Zilch::TypeCopyMode::ReferenceType);

  virtual void Initialize(const CompositionInitializer& initializer) override;
  void InitializeGraphics(const GraphicsEngineInitData& initData);
  void Shutdown();

  void Add(GraphicsSpace* space);
  void Remove(GraphicsSpace* space);

  void OnEngineUpdate(Zilch::EventData* e);
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
