#pragma once

#include "Helpers/Mesh.hpp"
#include "Helpers/Shader.hpp"
#include "Helpers/Material.hpp"
#include "Helpers/MaterialBinding.hpp"
#include "Helpers/Model.hpp"
#include "Helpers/Texture.hpp"
#include "VulkanRenderer.hpp"
#include <functional>

class GraphicsSpace;

struct GraphicsEngineRendererInitData
{
  SurfaceCreationDelegate mSurfaceCreationCallback;
  size_t mInitialWidth = 0;
  size_t mInitialHeight = 0;
};

struct GraphicsEngine
{
public:
  void Initialize();
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
  std::vector<GraphicsSpace*> mSpaces;

  MeshManager mMeshManager;
  TextureManager mTextureManager;
  ShaderManager mShaderManager;
  MaterialManager mMaterialManager;
  VulkanRenderer mRenderer;

  std::unordered_map<String, UniqueShaderMaterial> mUniqueShaderMaterials;
  std::unordered_map<Shader*, ShaderMaterialInstance> mShaderMaterialInstances;
};
