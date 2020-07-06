#include "Precompiled.hpp"

#include "VulkanSampler.hpp"

#include "VulkanStatus.hpp"

//-------------------------------------------------------------------VulkanSampler
VulkanSampler::VulkanSampler(VulkanSamplerCreationInfo& creationInfo)
{
  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.minFilter = creationInfo.mMinFilter;
  samplerInfo.magFilter = creationInfo.mMagFilter;
  samplerInfo.addressModeU = creationInfo.mAddressingU;
  samplerInfo.addressModeV = creationInfo.mAddressingV;
  samplerInfo.addressModeW = creationInfo.mAddressingW;
  samplerInfo.anisotropyEnable = creationInfo.mAnistropyEnabled;
  samplerInfo.maxAnisotropy = creationInfo.mMaxAnistropy;
  samplerInfo.borderColor = creationInfo.mBorderColor;
  samplerInfo.unnormalizedCoordinates = creationInfo.mUnnormalizedCoordinates;
  samplerInfo.compareEnable = creationInfo.mCompareMode;
  samplerInfo.compareOp = creationInfo.mCompareOp;
  samplerInfo.mipmapMode = creationInfo.mMipMapMode;
  samplerInfo.mipLodBias = creationInfo.mMipLodBias;
  samplerInfo.minLod = creationInfo.mMinLod;
  samplerInfo.maxLod = creationInfo.mMaxLod;

  if(vkCreateSampler(creationInfo.mDevice, &samplerInfo, nullptr, &mSampler) != VK_SUCCESS)
    VulkanStatus("failed to create texture sampler!");
  mInfo = creationInfo;
}

VulkanSampler::~VulkanSampler()
{
  Free();
}

void VulkanSampler::Free()
{
  if(mInfo.mDevice != VK_NULL_HANDLE)
    vkDestroySampler(mInfo.mDevice, mSampler, nullptr);

  mSampler = VK_NULL_HANDLE;
  mInfo = VulkanSamplerCreationInfo();
}

VkSampler VulkanSampler::GetVulkanSampler() const
{
  return mSampler;
}
