#pragma once

#include "Graphics/Texture.hpp"
#include "Graphics/ShaderEnumTypes.hpp"
#include "Graphics/MaterialShared.hpp"
#include "RenderPipelineEnums.hpp"

VkFormat GetImageFormat(TextureFormat format);
VkSamplerAddressMode ConvertSamplerAddressMode(TextureAddressing mode);
VkFilter ConvertFilterMode(TextureFiltering mode);
VkShaderStageFlags ConvertStageFlags(ShaderStageFlags::Enum flags);
VkDescriptorType ConvertDescriptorType(MaterialDescriptorType descriptorType);

VkBlendFactor ConvertBlendFactor(BlendFactor::Enum blendFactor);
VkBlendOp ConvertBlendEquation(BlendEquation::Enum blendEquation);
bool IsDepthReadEnabled(DepthMode::Enum depthMode);
bool IsDepthWriteEnabled(DepthMode::Enum depthMode);
VkCompareOp ConvertTextureCompareOp(TextureCompareFunc::Enum compareOp);
