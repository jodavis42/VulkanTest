#include "Precompiled.hpp"

#include "EnumConversions.hpp"

#include "Graphics/Texture.hpp"
#include "RenderPipelineSettings.hpp"

VkFormat GetImageFormat(TextureFormat format)
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

VkSamplerAddressMode ConvertSamplerAddressMode(TextureAddressing mode)
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

VkFilter ConvertFilterMode(TextureFiltering mode)
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

VkShaderStageFlags ConvertStageFlags(ShaderStageFlags::Enum flags)
{
  VkShaderStageFlags result = 0;
  if(flags & ShaderStageFlags::Vertex)
    result |= VK_SHADER_STAGE_VERTEX_BIT;
  if(flags & ShaderStageFlags::Pixel)
    result |= VK_SHADER_STAGE_FRAGMENT_BIT;
  return result;
}

VkDescriptorType ConvertDescriptorType(MaterialDescriptorType descriptorType)
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

VkBlendFactor ConvertBlendFactor(BlendFactor::Enum blendFactor)
{
  switch(blendFactor)
  {
  case BlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
  case BlendFactor::One: return VK_BLEND_FACTOR_ONE;
  case BlendFactor::SourceColor: return VK_BLEND_FACTOR_SRC_COLOR;
  case BlendFactor::InvSourceColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
  case BlendFactor::DestColor: return VK_BLEND_FACTOR_DST_COLOR;
  case BlendFactor::InvDestColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
  case BlendFactor::SourceAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
  case BlendFactor::InvSourceAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  case BlendFactor::DestAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
  case BlendFactor::InvDestAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
  case BlendFactor::SourceAlphaSaturage: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
  default: return VK_BLEND_FACTOR_ZERO;
  }
}

VkBlendOp ConvertBlendEquation(BlendEquation::Enum blendEquation)
{
  switch(blendEquation)
  {
  case BlendEquation::Add: return VK_BLEND_OP_ADD;
  case BlendEquation::Subtract: return VK_BLEND_OP_SUBTRACT;
  case BlendEquation::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
  case BlendEquation::Min: return VK_BLEND_OP_MIN;
  case BlendEquation::Max: return VK_BLEND_OP_MAX;
  default: return VK_BLEND_OP_ADD;
  }
}

bool IsDepthReadEnabled(DepthMode::Enum depthMode)
{
  switch(depthMode)
  {
  case DepthMode::Read:
  case DepthMode::ReadWrite:
    return VK_TRUE;
  default:
    return VK_FALSE;
  }
}

bool IsDepthWriteEnabled(DepthMode::Enum depthMode)
{
  switch(depthMode)
  {
  case DepthMode::Write:
  case DepthMode::ReadWrite:
    return VK_TRUE;
  default:
    return VK_FALSE;
  }
}

VkCompareOp ConvertTextureCompareOp(TextureCompareFunc::Enum compareOp)
{
  switch(compareOp)
  {
  case TextureCompareFunc::Never: return VK_COMPARE_OP_NEVER;
  case TextureCompareFunc::Always: return VK_COMPARE_OP_ALWAYS;
  case TextureCompareFunc::Less: return VK_COMPARE_OP_LESS;
  case TextureCompareFunc::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
  case TextureCompareFunc::Greater: VK_COMPARE_OP_GREATER;
  case TextureCompareFunc::GreaterEqual: VK_COMPARE_OP_GREATER_OR_EQUAL;
  case TextureCompareFunc::Equal: VK_COMPARE_OP_EQUAL;
  case TextureCompareFunc::NotEqual: VK_COMPARE_OP_NOT_EQUAL;
  default: return VK_COMPARE_OP_ALWAYS;
  }
}
