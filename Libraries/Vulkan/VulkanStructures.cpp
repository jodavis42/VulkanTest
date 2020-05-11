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
