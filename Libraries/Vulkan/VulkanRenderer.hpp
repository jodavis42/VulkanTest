#pragma once

#include "VulkanRendererInit.hpp"
#include "Math.hpp"
#include "Graphics/GraphicsBufferTypes.hpp"

#include "Graphics/Renderer.hpp"

struct Mesh;
struct Texture;
struct ZilchShader;
struct RenderQueue;

struct VulkanRuntimeData;
struct VulkanMesh;
struct VulkanShader;
struct VulkanImage;
struct VulkanRenderFrame;
struct VulkanShaderMaterial;
struct VulkanUniformBuffers;
class VulkanRenderer;

enum class RenderFrameStatus
{
  Success = 0,
  OutOfDate,
  SubOptimal,
  Error
};

class VulkanRenderer : public Renderer
{
public:
  VulkanRenderer();
  virtual ~VulkanRenderer();

  void Initialize(const VulkanInitializationData& initData);
  void Cleanup();
  void CleanupResources();
  virtual void Shutdown() override;
  virtual void Destroy() override;

  virtual void CreateMesh(const Mesh* mesh) override;
  virtual void DestroyMesh(const Mesh* mesh) override;

  virtual void CreateTexture(const Texture* texture) override;
  virtual void DestroyTexture(const Texture* texture) override;

  virtual void CreateShader(const ZilchShader* zilchShader) override;
  virtual void DestroyShader(const ZilchShader* zilchShader) override;

  virtual void CreateShaderMaterial(ZilchShader* shaderMaterial) override;
  virtual void UpdateShaderMaterialInstance(const ZilchShader* zilchShader, const ZilchMaterial* zilchMaterial) override;
  virtual void UploadShaderMaterialInstances(MaterialBatchUploadData& materialBatchUploadData) override;
  virtual void DestroyShaderMaterial(const ZilchShader* zilchShader) override;

  RenderFrameStatus BeginFrame();
  RenderFrameStatus EndFrame();
  virtual void DrawRenderQueue(RenderQueue& renderQueue) override;
  virtual void WaitForIdle() override;

  virtual void Reshape(size_t width, size_t height, float aspectRatio) override;
  virtual void GetShape(size_t& width, size_t& height, float& aspectRatio) const override;

  virtual Matrix4 BuildPerspectiveMatrix(float verticalFov, float aspectRatio, float nearDistance, float farDistance) const override;

  void* MapGlobalUniformBufferMemory(const String& bufferName, uint32_t bufferId);
  void* MapPerFrameUniformBufferMemory(const String& bufferName, uint32_t bufferId, uint32_t frameIndex);
  void UnMapGlobalUniformBufferMemory(const String& bufferName, uint32_t bufferId);
  void UnMapPerFrameUniformBufferMemory(const String& bufferName, uint32_t bufferId, uint32_t frameIndex);
  size_t AlignUniformBufferOffset(size_t offset);
  
//private:

  void DestroyMeshInternal(VulkanMesh* vulkanMesh);
  void DestroyTextureInternal(VulkanImage* vulkanImage);
  void DestroyShaderInternal(VulkanShader* vulkanShader);
  void DestroyShaderMaterialInternal(VulkanShaderMaterial* vulkanShaderMaterial);
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

  HashMap<const Mesh*, VulkanMesh*> mMeshMap;
  HashMap<const Texture*, VulkanImage*> mTextureMap;
  HashMap<String, VulkanImage*> mTextureNameMap;
  HashMap<const ZilchShader*, VulkanShader*> mZilchShaderMap;
  HashMap<const ZilchShader*, VulkanShaderMaterial*> mUniqueZilchShaderMaterialMap;
};
