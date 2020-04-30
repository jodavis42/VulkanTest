#pragma once

struct UniqueShaderMaterial;
struct ShaderMaterialInstance;
class VulkanRenderer;
struct VulkanRuntimeData;
struct VulkanShaderMaterial;
struct VulkanShader;

struct RendererData
{
  VulkanRenderer* mRenderer;
  VulkanRuntimeData* mRuntimeData;
};

void CreateMaterialDescriptorSetLayouts(RendererData& rendererData, const UniqueShaderMaterial& uniqueShaderMaterial, VulkanShaderMaterial& vulkanShaderMaterial);
void CreateMaterialDescriptorPool(RendererData& rendererData, const UniqueShaderMaterial& uniqueShaderMaterial, VulkanShaderMaterial& vulkanShaderMaterial);
void CreateMaterialDescriptorSets(RendererData& rendererData, VulkanShaderMaterial& vulkanShaderMaterial);
void UpdateMaterialDescriptorSet(RendererData& rendererData, const ShaderMaterialInstance& shaderMaterialInstance, VulkanShaderMaterial& vulkanShaderMaterial, size_t frameIndex, VkDescriptorSet descriptorSet);
void UpdateMaterialDescriptorSets(RendererData& rendererData, const ShaderMaterialInstance& shaderMaterialInstance, VulkanShaderMaterial& vulkanShaderMaterial);

void CreateGraphicsPipeline(RendererData& rendererData, const VulkanShader& vulkanShader, VulkanShaderMaterial& vulkanShaderMaterial);

//void DestroyVulkanPipeline(RendererData& rendererData, VulkanMaterialPipeline* vulkanPipeline);
