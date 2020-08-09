#pragma once

#include "VulkanStandard.hpp"
#include "VulkanRenderPass.hpp"
#include "Graphics/RenderPipelineSettings.hpp"

class RenderPassCache;
class VulkanPipeline;
class VulkanRenderer;
struct VulkanShader;
struct VulkanShaderMaterial;

//-------------------------------------------------------------------VulkanPipelineCacheInfo
struct VulkanPipelineCacheInfo
{
  size_t mRenderPassCookie = 0;
  const VulkanShader* mVulkanShader = nullptr;
  const VulkanShaderMaterial* mVulkanShaderMaterial = nullptr;
  const RenderPipelineSettings* mPipelineSettings = nullptr;
};

//-------------------------------------------------------------------VulkanMaterialPipelineCacheCreationInfo
struct VulkanMaterialPipelineCacheCreationInfo
{
  VkDevice mDevice = VK_NULL_HANDLE;
  RenderPassCache* mRenderPassCache = nullptr;
  VulkanRenderer* mRenderer = nullptr;
};

//-------------------------------------------------------------------VulkanMaterialPipelineCache
class VulkanMaterialPipelineCache
{
public:
  VulkanMaterialPipelineCache(const VulkanMaterialPipelineCacheCreationInfo& creationInfo);
  ~VulkanMaterialPipelineCache();

  VulkanPipeline* FindOrCreate(const VulkanPipelineCacheInfo& info);

  void Free();

private:
  struct MaterialEntryKey
  {
    size_t mRenderPassCookie = 0;
    size_t mPipelineHash = 0;

    size_t Hash() const;
    bool operator==(const MaterialEntryKey& rhs) const = default;
  };
  struct MaterialEntry
  {
    HashMap<MaterialEntryKey, VulkanPipeline*> mPipelines;
  };
  HashMap<const VulkanShaderMaterial*, MaterialEntry> mMaterialMap;

  VkDevice mDevice = VK_NULL_HANDLE;
  RenderPassCache* mRenderPassCache = nullptr;
  VulkanRenderer* mRenderer = nullptr;
};
