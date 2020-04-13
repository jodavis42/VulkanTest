#pragma once

struct VulkanMesh
{
  VkBuffer mVertexBuffer;
  VkDeviceMemory mVertexBufferMemory;

  VkBuffer mIndexBuffer;
  VkDeviceMemory mIndexBufferMemory;
  uint32_t mIndexCount;
};

struct VulkanMaterial
{
  VkShaderModule mVertexShaderModule;
  VkShaderModule mPixelShaderModule;
  VkDescriptorSetLayout mDescriptorSetLayout;
  uint32_t mBufferOffset;
  uint32_t mBufferSize;
};

struct VulkanVertex
{
  static std::vector<VkVertexInputBindingDescription> getBindingDescription()
  {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    bindingDescriptions.resize(1);

    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescriptions;
  }

  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
  {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    attributeDescriptions.resize(3);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, uv);

    return attributeDescriptions;
  }
};
