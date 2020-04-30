#pragma once

#include <vector.>

struct VulkanMesh
{
  VkBuffer mVertexBuffer;
  VkDeviceMemory mVertexBufferMemory;

  VkBuffer mIndexBuffer;
  VkDeviceMemory mIndexBufferMemory;
  uint32_t mIndexCount;
};

struct VulkanShader
{
  VkShaderModule mVertexShaderModule;
  VkShaderModule mPixelShaderModule;
};

//struct VulkanMaterial
//{
//  VkShaderModule mVertexShaderModule;
//  VkShaderModule mPixelShaderModule;
//  VkDescriptorSetLayout mDescriptorSetLayout;
//  uint32_t mBufferOffset;
//  uint32_t mBufferSize;
//};
//
//struct VulkanMaterialPipeline
//{
//  VkPipelineLayout mPipelineLayout;
//  VkPipeline mPipeline;
//  VkDescriptorPool mDescriptorPool;
//  std::vector<VkDescriptorSet> mDescriptorSets;
//};

struct VulkanShaderMaterial
{
  VkDescriptorSetLayout mDescriptorSetLayout;
  VkPipelineLayout mPipelineLayout;
  std::vector<VkDescriptorSet> mDescriptorSets;
  
  VkPipeline mPipeline;
  VkDescriptorPool mDescriptorPool;
  
  uint32_t mBufferId;
};

struct VulkanUniformBuffer
{
  VkBuffer mBuffer;
  VkDeviceMemory mBufferMemory;
  VkDeviceSize mUsedSize = 0;
  VkDeviceSize mAllocatedSize = 0;
};

struct VulkanUniformBuffers
{
  std::vector<VulkanUniformBuffer> mBuffers;
};

struct VulkanVertex
{
  static std::vector<VkVertexInputBindingDescription> getBindingDescription();
  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

struct VulkanImage
{
  VkImage mImage = VK_NULL_HANDLE;
  VkDeviceMemory mImageMemory = VK_NULL_HANDLE;
  VkImageView mImageView = VK_NULL_HANDLE;
  VkSampler mSampler = VK_NULL_HANDLE;
};

class VulkanRenderer;
struct VulkanRenderFrame
{
  VulkanRenderer* mRenderer;
  VkImage mSwapChainImage;
  VkImageView mSwapChainImageView;
  VkRenderPass mRenderPass;
  VkFramebuffer mFrameBuffer;
  VkCommandBuffer mCommandBuffer;

  VkDescriptorSet mDescriptorSet;
};
