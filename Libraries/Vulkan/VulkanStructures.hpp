#pragma once

#include "VulkanStandard.hpp"

#include "VulkanResourcePool.hpp"

struct VulkanRuntimeData;
class VulkanRenderer;
class VulkanSampler;
class VulkanImage;
class VulkanImageView;
class VulkanFrameBuffer;
class VulkanRenderer;
class VulkanCommandBuffer;
class VulkanRenderPass;
class VulkanPipeline;

struct RendererData
{
  VulkanRenderer* mRenderer;
  VulkanRuntimeData* mRuntimeData;
};

struct VulkanMesh
{
  VkBuffer mVertexBuffer;
  VkDeviceMemory mVertexBufferMemory;

  VkBuffer mIndexBuffer;
  VkDeviceMemory mIndexBufferMemory;
  uint32_t mIndexCount;
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

struct VulkanTexturedImageData
{
  VulkanImage* mImage = nullptr;
  VulkanImageView* mImageView = nullptr;
  VulkanSampler* mSampler = nullptr;
  VkDeviceMemory mImageMemory = VK_NULL_HANDLE;
};

struct VulkanRenderFrame
{
  VulkanRenderer* mRenderer = nullptr;
  VulkanCommandBuffer* mCommandBuffer = nullptr;

  VkDescriptorSet mDescriptorSet;

  VulkanResourcePool mResources;
};
