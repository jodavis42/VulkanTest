#include "Precompiled.hpp"

#include "Graphics/Vertex.hpp"

#include <vulkan/vulkan.h>
#include "VulkanStructures.hpp"
#include "VulkanBufferCreation.hpp"
#include "VulkanInitialization.hpp"

VulkanUniformBufferManager::~VulkanUniformBufferManager()
{
  Destroy();
}

void VulkanUniformBufferManager::Destroy()
{
  for(VulkanGlobalUniformBuffers& globalBuffer : mNamedGlobalBuffers.Values())
  {
    for(VulkanUniformBuffer& buffer : globalBuffer.mBuffersById.Values())
    {
      vkDestroyBuffer(mRuntimeData->mDevice, buffer.mBuffer, nullptr);
      vkFreeMemory(mRuntimeData->mDevice, buffer.mBufferMemory, nullptr);
    }
  }
  for(VulkanPerFrameBuffers& perFrameBuffer : mNamedPerFrameBuffers.Values())
  {
    for(VulkanPerFrameBuffers::FrameBuffers& frameBuffers: perFrameBuffer.mBuffersById.Values())
    {
      for(VulkanUniformBuffer& buffer : frameBuffers.mBuffers)
      {
        vkDestroyBuffer(mRuntimeData->mDevice, buffer.mBuffer, nullptr);
        vkFreeMemory(mRuntimeData->mDevice, buffer.mBufferMemory, nullptr);
      }
    }
  }
  mNamedGlobalBuffers.Clear();
  mNamedPerFrameBuffers.Clear();
}

VulkanPerFrameBuffers::FrameBuffers* VulkanUniformBufferManager::CreatePerFrameBuffer(const String& name, uint32_t bufferId)
{
  VulkanPerFrameBuffers::FrameBuffers& frameBuffers = mNamedPerFrameBuffers[name].mBuffersById[bufferId];

  if(frameBuffers.mBuffers.Empty())
  {
    uint32_t frameCount = mRuntimeData->mSwapChain.GetCount();
    frameBuffers.mBuffers.Resize(frameCount);
    VulkanBufferCreationData vulkanData{mRuntimeData->mPhysicalDevice, mRuntimeData->mDevice, mRuntimeData->mGraphicsQueue, mRuntimeData->mCommandPool};
    for(size_t i = 0; i < frameCount; i++)
    {
      VulkanUniformBuffer& buffer = frameBuffers.mBuffers[i];
      buffer.mAllocatedSize = mRuntimeData->mDeviceLimits.mMaxUniformBufferRange;
      buffer.mUsedSize = 0;
      VkImageUsageFlags usageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      CreateBuffer(vulkanData, buffer.mAllocatedSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, usageFlags, buffer.mBuffer, buffer.mBufferMemory);
    }
  }
  return &frameBuffers;
}

VulkanUniformBuffer* VulkanUniformBufferManager::FindPerFrameBuffer(const String& name, uint32_t bufferId, uint32_t frameId)
{
  VulkanPerFrameBuffers* perFrameBuffers = mNamedPerFrameBuffers.FindPointer(name);
  if(perFrameBuffers != nullptr)
  {
    VulkanPerFrameBuffers::FrameBuffers* frameBuffers = perFrameBuffers->mBuffersById.FindPointer(bufferId);
    if(frameBuffers != nullptr)
    {
      if(frameId < frameBuffers->mBuffers.Size())
      {
        return &frameBuffers->mBuffers[frameId];
      }
    }
  }
  return nullptr;
}

VulkanUniformBuffer* VulkanUniformBufferManager::FindOrCreatePerFrameBuffer(const String& name, uint32_t bufferId, uint32_t frameId)
{
  VulkanPerFrameBuffers::FrameBuffers* frameBuffers = CreatePerFrameBuffer(name, bufferId);
  return &frameBuffers->mBuffers[frameId];
}

uint32_t VulkanUniformBufferManager::GlobalBufferCount(const String& name)
{
  VulkanGlobalUniformBuffers* buffers = mNamedGlobalBuffers.FindPointer(name);
  if(buffers != nullptr)
    return static_cast<uint32_t>(buffers->mBuffersById.Size());
  return 0;
}

VulkanUniformBuffer* VulkanUniformBufferManager::CreateGlobalBuffer(const String& name, uint32_t bufferId)
{
  VulkanUniformBuffer& buffer = mNamedGlobalBuffers[name].mBuffersById[bufferId];
  buffer.mAllocatedSize = mRuntimeData->mDeviceLimits.mMaxUniformBufferRange;
  buffer.mUsedSize = 0;
  VulkanBufferCreationData vulkanData{mRuntimeData->mPhysicalDevice, mRuntimeData->mDevice, mRuntimeData->mGraphicsQueue, mRuntimeData->mCommandPool};
  VkImageUsageFlags usageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  CreateBuffer(vulkanData, buffer.mAllocatedSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, usageFlags, buffer.mBuffer, buffer.mBufferMemory);
  return &buffer;
}

VulkanUniformBuffer* VulkanUniformBufferManager::FindGlobalBuffer(const String& name, uint32_t bufferId)
{
  VulkanGlobalUniformBuffers* buffers = mNamedGlobalBuffers.FindPointer(name);
  if(buffers != nullptr)
  {
    return buffers->mBuffersById.FindPointer(bufferId);
  }
  return nullptr;
}

Array<VkVertexInputBindingDescription> VulkanVertex::getBindingDescription()
{
  Array<VkVertexInputBindingDescription> bindingDescriptions;
  bindingDescriptions.Resize(1);

  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescriptions;
}

Array<VkVertexInputAttributeDescription> VulkanVertex::getAttributeDescriptions()
{
  Array<VkVertexInputAttributeDescription> attributeDescriptions;
  attributeDescriptions.Reserve(5);

  VkVertexInputAttributeDescription& posDescription = attributeDescriptions.PushBack();
  posDescription.binding = 0;
  posDescription.location = static_cast<uint32_t>(attributeDescriptions.Size() - 1);
  posDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
  posDescription.offset = offsetof(Vertex, pos);

  VkVertexInputAttributeDescription& normalDescription = attributeDescriptions.PushBack();
  normalDescription.binding = 0;
  normalDescription.location = static_cast<uint32_t>(attributeDescriptions.Size() - 1);
  normalDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
  normalDescription.offset = offsetof(Vertex, normal);

  VkVertexInputAttributeDescription& colorDescription = attributeDescriptions.PushBack();
  colorDescription.binding = 0;
  colorDescription.location = static_cast<uint32_t>(attributeDescriptions.Size() - 1);
  colorDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  colorDescription.offset = offsetof(Vertex, color);

  VkVertexInputAttributeDescription& uvDescription = attributeDescriptions.PushBack();
  uvDescription.binding = 0;
  uvDescription.location = static_cast<uint32_t>(attributeDescriptions.Size() - 1);
  uvDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
  uvDescription.offset = offsetof(Vertex, uv);

  VkVertexInputAttributeDescription& aux0Description = attributeDescriptions.PushBack();
  aux0Description.binding = 0;
  aux0Description.location = static_cast<uint32_t>(attributeDescriptions.Size() - 1);
  aux0Description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
  aux0Description.offset = offsetof(Vertex, aux0);

  return attributeDescriptions;
}
