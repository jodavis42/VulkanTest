#pragma once

#include "Mesh.hpp"
#include "Shader.hpp"
#include "Material.hpp"
#include "MaterialBinding.hpp"
#include "Model.hpp"
#include "Texture.hpp"
#include "VulkanRenderer.hpp"
#include <functional>

class GraphicsSpace;

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
};

struct GraphicsEngine
{
public:
  void Initialize(const GraphicsEngineInitData& initData);
  void Shutdown();

  GraphicsSpace* CreateSpace(const String& name);
  GraphicsSpace* FindSpace(const String& name);
  void DestroySpace(const String& name);
  void DestroySpace(GraphicsSpace* space);

  void LoadShadersAndMaterials();
  void InitializeRenderer(GraphicsEngineRendererInitData& rendererInitData);
  void LoadVulkanImages();
  void LoadVulkanShaders();
  void LoadVulkanMaterial(Material* material);
  void LoadVulkanMaterials();
  void LoadVulkanMeshes();

  void PopulateMaterialBuffer();
  void CleanupSwapChain();
  void RecreateSwapChain();
  void WaitIdle();

  SurfaceCreationDelegate mSurfaceCreationCallback;
  std::function<void(size_t&, size_t&)> mWindowSizeQueryFn = nullptr;
  Array<GraphicsSpace*> mSpaces;

  MeshManager mMeshManager;
  TextureManager mTextureManager;
  ShaderManager mShaderManager;
  MaterialManager mMaterialManager;
  VulkanRenderer mRenderer;

  HashMap<String, UniqueShaderMaterial> mUniqueShaderMaterialNameMap;
  HashMap<String, ShaderMaterialInstance> mShaderMaterialInstanceNameMap;
};
