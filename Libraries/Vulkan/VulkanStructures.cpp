#include "Precompiled.hpp"

#include "Graphics/Vertex.hpp"

#include <vulkan/vulkan.h>
#include "VulkanStructures.hpp"


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
  attributeDescriptions.Resize(3);

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
