#pragma once

class VulkanRenderer;
struct VulkanRuntimeData;
struct Material;
struct ShaderBinding;
struct ShaderMaterialBinding;
struct VulkanShaderMaterial;
struct VulkanShader;

struct RendererData
{
  VulkanRenderer* mRenderer;
  VulkanRuntimeData* mRuntimeData;
};

void CreateMaterialDescriptorSetLayouts(RendererData& rendererData, const ShaderBinding& shaderMaterial, VulkanShaderMaterial& vulkanShaderMaterial);
void CreateMaterialDescriptorPool(RendererData& rendererData, const ShaderBinding& shaderMaterial, VulkanShaderMaterial& vulkanShaderMaterial);
void CreateMaterialDescriptorSets(RendererData& rendererData, const ShaderBinding& shaderMaterial, VulkanShaderMaterial& vulkanShaderMaterial);
void UpdateMaterialDescriptorSet(RendererData& rendererData, const ShaderMaterialBinding& shaderMaterialBinding, VulkanShaderMaterial& vulkanShaderMaterial, size_t frameIndex, VkDescriptorSet descriptorSet);
void UpdateMaterialDescriptorSets(RendererData& rendererData, const ShaderMaterialBinding& shaderMaterialBinding, VulkanShaderMaterial& vulkanShaderMaterial);

void CreateGraphicsPipeline(RendererData& rendererData, const VulkanShader& vulkanShader, VulkanShaderMaterial& vulkanShaderMaterial);

//void DestroyVulkanPipeline(RendererData& rendererData, VulkanMaterialPipeline* vulkanPipeline);
