#pragma once

#include <unordered_map>
#include "VulkanRendererInit.hpp"
#include "Helpers/Math.hpp"

struct VulkanRuntimeData;
struct Mesh;
struct Texture;
struct Shader;
struct Material;
struct ShaderBinding;
struct ShaderMaterialBinding;

struct VulkanMesh;
struct VulkanShader;
struct VulkanImage;
struct RenderFrame;
struct VulkanRenderFrame;
class VulkanRenderer;
struct RenderFrame;
struct VulkanShaderMaterial;

template <typename T>
struct Factory
{
  struct Slot
  {
    T* mItem = nullptr;
    bool mValid = false;
  };
  std::vector<Slot> mSlots;
};

struct RenderTarget
{
  RenderFrame* mRenderFrame = nullptr;
  size_t mId = 0;
};

struct RenderPass
{
  RenderTarget* mTarget = nullptr;
  RenderFrame* mRenderFrame = nullptr;
  size_t mId = 0;
};

struct CommandBuffer
{
  void Begin();
  void End();
  void BeginRenderPass(RenderPass* renderPass);
  void EndRenderPass(RenderPass* renderPass);

  RenderFrame* mRenderFrame = nullptr;
  size_t mId = 0;
};

struct RenderFrame
{
  RenderFrame(VulkanRenderer* renderer, size_t id);
  RenderPass* GetFinalRenderPass();

  RenderTarget* GetFinalRenderTarget();
  RenderTarget* CreateRenderTarget(Integer2 size);

  CommandBuffer* GetFinalCommandBuffer();
  CommandBuffer* CreateCommandBuffer();

  VulkanRenderer* mRenderer = nullptr;
  size_t mId = 0;


  // Private
  CommandBuffer mCommandBuffer;
  RenderTarget mRenderTarget;
  RenderPass mRenderPass;

};

class VulkanRenderer
{
public:
  VulkanRenderer();
  ~VulkanRenderer();

  void Initialize(const VulkanInitializationData& initData);
  void Cleanup();
  void CleanupResources();
  void Destroy();

  void CreateMesh(const Mesh* mesh);
  void DestroyMesh(const Mesh* mesh);

  void CreateTexture(const Texture* texture);
  void DestroyTexture(const Texture* texture);

  void CreateShader(const Shader* shader);
  void DestroyShader(const Shader* shader);

  void CreateShaderMaterial(const ShaderBinding* shaderMaterial);
  void UpdateShaderMaterial(const ShaderMaterialBinding* shaderMaterialBinding);
  void DestroyShaderMaterial(const ShaderBinding* shaderMaterial);

  RenderFrame* BeginFrame();
  void EndFrame(RenderFrame* frame);
  
  void Resize(size_t width, size_t height);

  //
  //virtual BufferRenderData CreateBuffer(BufferCreationData& creationData, BufferType::Enum bufferType) { return BufferRenderData(); };
  //virtual void UploadBuffer(BufferRenderData& renderData, ByteBuffer& data) {};
  //virtual void* MapBuffer(BufferRenderData& renderData, size_t offset, size_t sizeInBytes, BufferMappingType::Enum mappingTypes) { return nullptr; };
  //virtual void UnMapBuffer(BufferRenderData& renderData) {};
  //virtual void DestroyBuffer(BufferRenderData& renderData) {};
  //
  //virtual void ClearTarget() {};
  //virtual void Draw(ObjectData& objData) {};
  //virtual void DispatchCompute(ObjectData& objData, int x, int y, int z) {};
  //
  //virtual void Reshape(int width, int height, float aspectRatio) {};
  virtual Matrix4 BuildPerspectiveMatrix(float verticalFov, float aspectRatio, float nearDistance, float farDistance);
  //virtual ZilchShaderIRBackend* CreateBackend() abstract;
  void Draw();

  VulkanRuntimeData* GetRuntimeData(){return mInternal;}
//private:

  void RecreateFramesInternal();
  void CreateSwapChainInternal();
  void DestroySwapChainInternal();
  void CreateRenderFramesInternal();
  void DestroyRenderFramesInternal();
  void CreateImageInternal(const Texture* texture, VulkanImage* image);
  void CreateImageViewInternal(const Texture* texture, VulkanImage* image);
  void CreateDepthResourcesInternal();
  void DestroyDepthResourcesInternal();

  VulkanRuntimeData* mInternal;

  std::unordered_map<const Mesh*, VulkanMesh*> mMeshMap;
  std::unordered_map<const Texture*, VulkanImage*> mTextureMap;
  std::unordered_map<String, VulkanImage*> mTextureNameMap;
  std::unordered_map<const Shader*, VulkanShader*> mShaderMap;
  std::unordered_map<const ShaderBinding*, VulkanShaderMaterial*> mShaderMaterialMap;
};
