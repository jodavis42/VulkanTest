#pragma once

//-------------------------------------------------------------------VulkanSamplerCreationInfo
struct VulkanSamplerCreationInfo
{
  VkDevice mDevice = VK_NULL_HANDLE;
  VkSamplerCreateFlags mFlags = 0;
  VkFilter mMinFilter = VK_FILTER_LINEAR;
  VkFilter mMagFilter = VK_FILTER_LINEAR;
  VkSamplerMipmapMode mMipMapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  VkSamplerAddressMode mAddressingU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  VkSamplerAddressMode mAddressingV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  VkSamplerAddressMode mAddressingW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  float mMipLodBias = 0.0f;
  float mMinLod = 0.0f;
  float mMaxLod = 1.0f;
  VkBool32 mAnistropyEnabled = VK_TRUE;
  float mMaxAnistropy = 16;
  VkBool32 mCompareMode = VK_FALSE;
  VkCompareOp mCompareOp = VK_COMPARE_OP_ALWAYS;
  VkBorderColor mBorderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  VkBool32 mUnnormalizedCoordinates = VK_FALSE;
};

//-------------------------------------------------------------------VulkanSampler
class VulkanSampler
{
public:
  VulkanSampler(VulkanSamplerCreationInfo& creationInfo);

  VulkanSampler(VulkanSampler&&) = delete;
  VulkanSampler& operator=(VulkanSampler&&) = delete;
  ~VulkanSampler();

  void Free();

  VkSampler GetVulkanSampler() const;

private:
  VkSampler mSampler = VK_NULL_HANDLE;
  VulkanSamplerCreationInfo mInfo;
};
