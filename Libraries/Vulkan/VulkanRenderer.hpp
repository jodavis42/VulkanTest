#pragma once

#include <unordered_map>
#include "VulkanRendererInit.hpp"
#include "Math.hpp"
#include "Graphics/GraphicsBufferTypes.hpp"

struct VulkanRuntimeData;
struct Mesh;
struct Texture;
struct UniqueShaderMaterial;
struct ShaderMaterialInstance;
struct ZilchShader;
struct ZilchMaterial;
struct Model;

struct VulkanMesh;
struct VulkanShader;
struct VulkanImage;
struct RenderFrame;
struct VulkanRenderFrame;
struct RenderFrame;
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

template <typename T>
struct Factory
{
  struct Slot
  {
    T* mItem = nullptr;
    bool mValid = false;
  };
  Array<Slot> mSlots;
};

struct RenderTarget
{
  RenderFrame* mRenderFrame = nullptr;
  uint32_t mId = 0;
};

struct RenderPass
{
  RenderTarget* mTarget = nullptr;
  RenderFrame* mRenderFrame = nullptr;
  uint32_t mId = 0;
};

struct CommandBuffer
{
  void Begin();
  void End();
  void BeginRenderPass(RenderPass* renderPass);
  void EndRenderPass(RenderPass* renderPass);

  RenderFrame* mRenderFrame = nullptr;
  uint32_t mId = 0;
};

struct RenderFrame
{
  RenderFrame(VulkanRenderer* renderer, uint32_t id);
  RenderPass* GetFinalRenderPass();

  RenderTarget* GetFinalRenderTarget();
  RenderTarget* CreateRenderTarget(Integer2 size);

  CommandBuffer* GetFinalCommandBuffer();
  CommandBuffer* CreateCommandBuffer();

  VulkanRenderer* mRenderer = nullptr;
  uint32_t mId = 0;


  // Private
  CommandBuffer mCommandBuffer;
  RenderTarget mRenderTarget;
  RenderPass mRenderPass;
};

struct ModelRenderData
{
  const Model* mModel = nullptr;
  const Mesh* mMesh = nullptr;
  const ZilchMaterial* mZilchMaterial = nullptr;
  const ZilchShader* mZilchShader = nullptr;
  Matrix4 mTransform;
};

struct RenderBatchDrawData
{
  FrameData mFrameData;
  CameraData mCameraData;
  Matrix4 mWorldToView;
  Matrix4 mViewToPerspective;
};

class VulkanRenderer
{
public:
  VulkanRenderer();
  ~VulkanRenderer();

  void Initialize(const VulkanInitializationData& initData);
  void Cleanup();
  void CleanupResources();
  void Shutdown();
  void Destroy();

  void CreateMesh(const Mesh* mesh);
  void DestroyMesh(const Mesh* mesh);

  void CreateTexture(const Texture* texture);
  void DestroyTexture(const Texture* texture);

  void CreateShader(const ZilchShader* zilchShader);
  void DestroyShader(const ZilchShader* zilchShader);

  void CreateShaderMaterial(ZilchShader* shaderMaterial);
  void UpdateShaderMaterialInstance(const ZilchShader* zilchShader, const ZilchMaterial* zilchMaterial);
  void DestroyShaderMaterial(const ZilchShader* zilchShader);

  RenderFrameStatus BeginFrame(RenderFrame*& frame);
  RenderFrameStatus EndFrame(RenderFrame*& frame);
  void QueueDraw(const ModelRenderData& renderData);
  void Draw(const RenderBatchDrawData& batchDrawData);
  
  void Resize(size_t width, size_t height);
  void GetSize(size_t& width, size_t& height);

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

  void* MapGlobalUniformBufferMemory(const String& bufferName, uint32_t bufferId);
  void* MapPerFrameUniformBufferMemory(const String& bufferName, uint32_t bufferId, uint32_t frameIndex);
  void UnMapGlobalUniformBufferMemory(const String& bufferName, uint32_t bufferId);
  void UnMapPerFrameUniformBufferMemory(const String& bufferName, uint32_t bufferId, uint32_t frameIndex);
  size_t AlignUniformBufferOffset(size_t offset);

  VulkanRuntimeData* GetRuntimeData(){return mInternal;}
  
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
  Array<ModelRenderData> mModelRenderData;
  RenderFrame* mCurrentFrame = nullptr;
};
