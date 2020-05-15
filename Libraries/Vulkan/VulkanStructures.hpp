#pragma once

#include "VulkanStandard.hpp"

struct VulkanRuntimeData;

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
  
  uint32_t mBufferId = 0;
  size_t mBufferOffset = 0;
};

struct VulkanUniformBuffer
{
  VkBuffer mBuffer;
  VkDeviceMemory mBufferMemory;
  VkDeviceSize mUsedSize = 0;
  VkDeviceSize mAllocatedSize = 0;
};

struct VulkanGlobalUniformBuffers
{
  HashMap<uint32_t, VulkanUniformBuffer> mBuffersById;
};

struct VulkanPerFrameBuffers
{
  struct FrameBuffers
  {
    Array<VulkanUniformBuffer> mBuffers;
  };
  HashMap<uint32_t, FrameBuffers> mBuffersById;
};

struct VulkanUniformBufferManager
{
  ~VulkanUniformBufferManager();
  void Destroy();

  VulkanPerFrameBuffers::FrameBuffers* CreatePerFrameBuffer(const String& name, uint32_t bufferId);
  VulkanUniformBuffer* FindPerFrameBuffer(const String& name, uint32_t bufferId, uint32_t frameId);
  VulkanUniformBuffer* FindOrCreatePerFrameBuffer(const String& name, uint32_t bufferId, uint32_t frameId);

  uint32_t GlobalBufferCount(const String& name);
  VulkanUniformBuffer* CreateGlobalBuffer(const String& name, uint32_t bufferId);
  VulkanUniformBuffer* FindGlobalBuffer(const String& name, uint32_t bufferId);
  

  HashMap<String, VulkanPerFrameBuffers> mNamedPerFrameBuffers;
  HashMap<String, VulkanGlobalUniformBuffers> mNamedGlobalBuffers;
  VulkanRuntimeData* mRuntimeData = nullptr;
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
