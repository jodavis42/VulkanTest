#pragma once

struct ZilchShader;
struct ZilchMaterial;
struct ZilchShaderManager;
struct ZilchMaterialManager;
struct MaterialBatchUploadData;

class VulkanRenderer;
struct MaterialUploadData;
struct VulkanRuntimeData;
struct VulkanShaderMaterial;
struct VulkanShader;
struct RendererData;

void CreateMaterialDescriptorSetLayouts(RendererData& rendererData, const ZilchShader& zilchShader, VulkanShaderMaterial& vulkanShaderMaterial);
void CreateMaterialDescriptorPool(RendererData& rendererData, const ZilchShader& zilchShader, VulkanShaderMaterial& vulkanShaderMaterial);
void CreateMaterialDescriptorSets(RendererData& rendererData, VulkanShaderMaterial& vulkanShaderMaterial);
void UpdateMaterialDescriptorSet(RendererData& rendererData, const ZilchShader& zilchShader, const ZilchMaterial& zilchMaterial, VulkanShaderMaterial& vulkanShaderMaterial, size_t frameIndex, VkDescriptorSet descriptorSet);
void UpdateMaterialDescriptorSets(RendererData& rendererData, const ZilchShader& zilchShader, const ZilchMaterial& zilchMaterial, VulkanShaderMaterial& vulkanShaderMaterial);

void CreateGraphicsPipeline(RendererData& rendererData, const VulkanShader& vulkanShader, VulkanShaderMaterial& vulkanShaderMaterial);
void PopulateMaterialBuffers(RendererData& rendererData, MaterialBatchUploadData& materialBatchData);

//void DestroyVulkanPipeline(RendererData& rendererData, VulkanMaterialPipeline* vulkanPipeline);
