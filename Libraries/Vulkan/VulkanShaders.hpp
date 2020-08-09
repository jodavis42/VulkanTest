#pragma once

#include "VulkanStandard.hpp"

VkShaderModule CreateShaderModule(VkDevice& device, const uint32_t* byteCode, size_t byteCountSizeInBytes);
VkShaderModule CreateShaderModule(VkDevice& device, const Array<uint32_t>& code);
VkShaderModule CreateShaderModule(VkDevice& device, const Array<char>& code);

struct VulkanShader
{
  VkShaderModule mVertexShaderModule;
  VkShaderModule mPixelShaderModule;
  String mVertexEntryPointName;
  String mPixelEntryPointName;
};

struct VulkanShaderMaterial
{
  VkDescriptorSetLayout mDescriptorSetLayout;
  Array<VkDescriptorSet> mDescriptorSets;
  VkDescriptorPool mDescriptorPool;

  uint32_t mBufferId = 0;
  size_t mBufferOffset = 0;
};
