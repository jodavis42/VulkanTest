#include "Precompiled.hpp"

#include "VulkanShaders.hpp"

VkShaderModule CreateShaderModule(VkDevice& device, const uint32_t* byteCode, size_t byteCountSizeInBytes)
{
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = byteCountSizeInBytes;
  createInfo.pCode = byteCode;

  VkShaderModule shaderModule;
  VkResult vkResult = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
  ErrorIf(vkResult != VK_SUCCESS, "failed to create shader module!");

  return shaderModule;
}

VkShaderModule CreateShaderModule(VkDevice& device, const Array<uint32_t>& code)
{
  return CreateShaderModule(device, code.Data(), code.Size() * sizeof(uint32_t));
}

VkShaderModule CreateShaderModule(VkDevice& device, const Array<char>& code)
{
  const uint32_t* codePtr = reinterpret_cast<const uint32_t*>(code.Data());
  return CreateShaderModule(device, codePtr, code.Size());
}
