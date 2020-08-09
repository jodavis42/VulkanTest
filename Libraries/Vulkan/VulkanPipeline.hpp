#pragma once

#include "VulkanStandard.hpp"

class VulkanRenderPass;
struct VulkanShader;
struct VulkanShaderMaterial;
struct RenderPipelineSettings;

//-------------------------------------------------------------------VulkanPipelineCreationInfo
struct VulkanPipelineCreationInfo
{
  const VulkanRenderPass* mRenderPass = nullptr;
  const RenderPipelineSettings* mPipelineSettings = nullptr;
  const VulkanShader* mShader = nullptr;
  const VulkanShaderMaterial* mShaderMaterial = nullptr;

  Vec2 mViewportOffset = Vec2(0, 0);
  Vec2 mViewportSize = Vec2(0, 0);

  Array<VkVertexInputBindingDescription> mVertexBindingDescriptions;
  Array<VkVertexInputAttributeDescription> mVertexAttributeDescriptions;
  Array<VkDescriptorSetLayout> mDescriptorSetLayouts;
};

//-------------------------------------------------------------------VulkanPipeline
class VulkanPipeline
{
public:
  VulkanPipeline(VkDevice device, const VulkanPipelineCreationInfo& creationInfo);
  ~VulkanPipeline();

  void Free();

  VkPipelineLayout GetVulkanPipelineLayout() const;
  VkPipeline GetVulkanPipeline() const;

private:
  void CreateVulkanPipelineLayout(const VulkanPipelineCreationInfo& creationInfo);
  void CreateVulkanPipeline(const VulkanPipelineCreationInfo& creationInfo);

  VkDevice mDevice = VK_NULL_HANDLE;
  VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
  VkPipeline mPipeline = VK_NULL_HANDLE;
};
