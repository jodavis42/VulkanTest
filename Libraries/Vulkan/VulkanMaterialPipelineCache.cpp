#include "Precompiled.hpp"

#include "VulkanMaterialPipelineCache.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanRenderPassCache.hpp"
#include "VulkanShaders.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanMaterials.hpp"
#include "VulkanInitialization.hpp"
#include "Utilities/Hasher.hpp"

//-------------------------------------------------------------------VulkanMaterialPipelineCache
VulkanMaterialPipelineCache::VulkanMaterialPipelineCache(const VulkanMaterialPipelineCacheCreationInfo& creationInfo)
{
  mDevice = creationInfo.mDevice;
  mRenderer = creationInfo.mRenderer;
  mRenderPassCache = creationInfo.mRenderPassCache;
}

VulkanMaterialPipelineCache::~VulkanMaterialPipelineCache()
{
  Free();
}

VulkanPipeline* VulkanMaterialPipelineCache::FindOrCreate(const VulkanPipelineCacheInfo& info)
{
  auto&& materialEntry = mMaterialMap[info.mVulkanShaderMaterial];

  MaterialEntryKey materialKey;
  materialKey.mPipelineHash = info.mPipelineSettings->Hash();
  materialKey.mRenderPassCookie = info.mRenderPassCookie;
  VulkanPipeline* pipeline = materialEntry.mPipelines.FindValue(materialKey, nullptr);
  if(pipeline != nullptr)
    return pipeline;

  VulkanRuntimeData* runtimeData = mRenderer->mInternal;
  VkExtent2D extent = runtimeData->mSwapChain->GetExtent();
  VulkanRenderPass* renderPass = mRenderPassCache->FindCompatible(RenderPassCache::RenderPassCookie(info.mRenderPassCookie));
  ErrorIf(renderPass == nullptr, "Couldn't find compatible render pass. This should never happen as the render pass should've already been created in order to get its cookie");

  VulkanPipelineCreationInfo creationInfo;
  creationInfo.mPipelineSettings = info.mPipelineSettings;
  creationInfo.mShader = info.mVulkanShader;
  creationInfo.mShaderMaterial = info.mVulkanShaderMaterial;
  creationInfo.mRenderPass = renderPass;
  creationInfo.mViewportSize = Vec2((float)extent.width, (float)extent.height);
  creationInfo.mVertexAttributeDescriptions = VulkanVertex::getAttributeDescriptions();
  creationInfo.mVertexBindingDescriptions = VulkanVertex::getBindingDescription();
  creationInfo.mDescriptorSetLayouts.PushBack(info.mVulkanShaderMaterial->mDescriptorSetLayout);

  pipeline = new VulkanPipeline(mDevice, creationInfo);
  materialEntry.mPipelines[materialKey] = pipeline;

  return pipeline;
}

void VulkanMaterialPipelineCache::Free()
{
  for(auto&& materialEntry : mMaterialMap.Values())
  {
    for(auto&& pipeline : materialEntry.mPipelines.Values())
    {
      delete pipeline;
    }
  }
  mMaterialMap.Clear();
}

size_t VulkanMaterialPipelineCache::MaterialEntryKey::Hash() const
{
  Hasher hasher;
  hasher.U64(mRenderPassCookie);
  hasher.U64(mPipelineHash);
  return hasher.mHash;
}
