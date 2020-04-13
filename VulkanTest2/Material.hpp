#pragma once

#include "VulkanStatus.hpp"

struct Material
{
  String mVertexShaderName;
  String mPixelShaderName;
};

struct VulkanMaterial
{
  VkShaderModule mVertexShaderModule;
  VkShaderModule mPixelShaderModule;
  VkDescriptorSetLayout mDescriptorSetLayout;
  uint32_t mBufferOffset;
  uint32_t mBufferSize;
};
