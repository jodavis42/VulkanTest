#pragma once

#include <unordered_map>
#include "VulkanRendererInit.hpp"

struct VulkanRuntimeData;
struct Mesh;
struct Texture;
struct Shader;
struct Material;
struct MaterialDescriptorSetLayout;

struct VulkanMesh;
struct VulkanMaterial;
struct VulkanShader;
struct VulkanImage;

class VulkanRenderer
{
public:
  VulkanRenderer();
  ~VulkanRenderer();

  void Initialize(const VulkanInitializationData& initData);
  void Cleanup();
  void Destroy();

  void CreateMesh(Mesh* mesh);
  void DestroyMesh(Mesh* mesh);

  void CreateTexture(Texture* texture);
  void DestroyTexture(Texture* texture);

  void CreateShader(Shader* shader);
  void DestroyShader(Shader* shader);

  void CreateMaterial(Material* material, MaterialDescriptorSetLayout* globalDescriptors, size_t globalCount);
  void DestroyMaterial(Material* material);
  
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
  //virtual Matrix4 BuildPerspectiveMatrix(float verticalFov, float aspectRatio, float nearDistance, float farDistance) abstract;
  //virtual ZilchShaderIRBackend* CreateBackend() abstract;
  void Draw();

  VulkanRuntimeData* GetRuntimeData(){return mInternal;}
//private:

  void CreateSwapChainInternal();
  void DestroySwapChainInternal();
  void CreateImageInternal(Texture* texture, VulkanImage* image);
  void CreateImageViewInternal(Texture* texture, VulkanImage* image);
  void SetDescriptorSetLayoutBinding(MaterialDescriptorSetLayout* materialDescriptor, void* vulkanDescriptor);
  void CreateDepthResourcesInternal();
  void DestroyDepthResourcesInternal();

  VulkanRuntimeData* mInternal;

  std::unordered_map<Mesh*, VulkanMesh*> mMeshMap;
  std::unordered_map<Texture*, VulkanImage*> mTextureMap;
  std::unordered_map<Shader*, VulkanShader*> mShaderMap;
  std::unordered_map<Material*, VulkanMaterial*> mMaterialMap;
};
