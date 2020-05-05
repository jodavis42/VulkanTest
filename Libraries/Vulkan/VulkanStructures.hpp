#pragma once

#include "VulkanStandard.hpp"

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
  String mVertexEntryPointName;
  String mPixelEntryPointName;
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
//  Array<VkDescriptorSet> mDescriptorSets;
//};

struct VulkanShaderMaterial
{
  VkDescriptorSetLayout mDescriptorSetLayout;
  VkPipelineLayout mPipelineLayout;
  Array<VkDescriptorSet> mDescriptorSets;
  
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
  Array<VulkanUniformBuffer> mBuffers;
};

struct VulkanVertex
{
  static Array<VkVertexInputBindingDescription> getBindingDescription();
  static Array<VkVertexInputAttributeDescription> getAttributeDescriptions();
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
