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
};
