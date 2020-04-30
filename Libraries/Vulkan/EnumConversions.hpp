#pragma once

#include "Graphics/Texture.hpp"

inline VkFormat GetImageFormat(TextureFormat format)
{
  switch(format)
  {
  case TextureFormat::SRGB8A8:
    return VK_FORMAT_R8G8B8A8_SRGB;
  case TextureFormat::RGBA32f:
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  default:
    return VK_FORMAT_UNDEFINED;
  }
}

inline VkSamplerAddressMode ConvertSamplerAddressMode(TextureAddressing mode)
{
  switch(mode)
  {
  case TextureAddressing::Clamp:
    return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  case TextureAddressing::Mirror:
    return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
  case TextureAddressing::Repeat:
    return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
  default:
    return VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
  }
}

inline VkFilter ConvertFilterMode(TextureFiltering mode)
{
  switch(mode)
  {
  case TextureFiltering::None:
    return VkFilter::VK_FILTER_NEAREST;
  case TextureFiltering::Nearest:
    return VkFilter::VK_FILTER_NEAREST;
  case TextureFiltering::Bilinear:
    return VkFilter::VK_FILTER_LINEAR;
  case TextureFiltering::Trilinear:
    return VkFilter::VK_FILTER_CUBIC_IMG;
  default:
    return VkFilter::VK_FILTER_MAX_ENUM;
  }
}

inline VkShaderStageFlags ConvertStageFlags(ShaderStageFlags::Enum flags)
{
  VkShaderStageFlags result = 0;
  if(flags & ShaderStageFlags::Vertex)
    result |= VK_SHADER_STAGE_VERTEX_BIT;
  if(flags & ShaderStageFlags::Pixel)
    result |= VK_SHADER_STAGE_FRAGMENT_BIT;
  return result;
}

inline VkDescriptorType ConvertDescriptorType(MaterialDescriptorType descriptorType)
{
  switch(descriptorType)
  {
  case MaterialDescriptorType::Uniform:
    return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  case MaterialDescriptorType::UniformDynamic:
    return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
  case MaterialDescriptorType::SampledImage:
    return VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  default:
    return VkDescriptorType::VK_DESCRIPTOR_TYPE_MAX_ENUM;
  }
}
