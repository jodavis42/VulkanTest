#include "Precompiled.hpp"

#include "VulkanImageCache.hpp"

#include "VulkanStatus.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanInitialization.hpp"
#include "VulkanMemoryAllocator.hpp"
#include "Utilities/Hasher.hpp"

//-------------------------------------------------------------------VulkanImageCache
VulkanImageCache::VulkanImageCache(const VulkanImageCacheCreationInfo& creationInfo)
{
  mInfo = creationInfo;
}

VulkanImageCache::~VulkanImageCache()
{
  Free();
}

void VulkanImageCache::Free()
{
  Clear();
}

VulkanImage* VulkanImageCache::FindOrCreateImage(const VulkanImageCreationInfo& imageCreationInfo)
{
  ImageCacheKey key(imageCreationInfo);
  ImageArray* images = mImages.FindPointer(key);
  if(images != nullptr)
  {
    VulkanImage* result = images->Back().mImage;
    images->PopBack();
    return result;
  }

  return CreateImage(imageCreationInfo);
}

VulkanImage* VulkanImageCache::CreateImage(const VulkanImageCreationInfo& imageCreationInfo)
{
  VkDevice device = mInfo.mDevice;
  VulkanRuntimeData* runtimeData = mInfo.mRenderer->mInternal;
  VulkanMemoryAllocator* memoryAllocator = runtimeData->mAllocator;
  ImageCacheKey key(imageCreationInfo);

  VulkanImageCreationInfo info = imageCreationInfo;
  info.mInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VulkanImage* vulkanImage = new VulkanImage(info);

  VkDeviceMemory imageMemory = memoryAllocator->AllocateImageMemory(vulkanImage, false);
  vkBindImageMemory(device, vulkanImage->GetVulkanImage(), imageMemory, 0);
  
  ImageLayoutTransitionInfo transitionInfo;
  transitionInfo.mDevice = device;
  transitionInfo.mGraphicsQueue = runtimeData->mGraphicsQueue;
  transitionInfo.mCommandPool = runtimeData->mCommandPool;
  transitionInfo.mFormat = imageCreationInfo.mFormat;
  transitionInfo.mImage = vulkanImage->GetVulkanImage();
  transitionInfo.mOldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  transitionInfo.mNewLayout = imageCreationInfo.mInitialLayout;
  transitionInfo.mMipLevels = imageCreationInfo.mMipLevels;
  TransitionImageLayout(transitionInfo);

  return vulkanImage;
}

void VulkanImageCache::ReleaseImage(VulkanImage* image)
{
  ImageCacheKey key(image->GetCreationInfo());
  CachedImage cachedImage;
  cachedImage.mImage = image;
  mImages[key].PushBack(cachedImage);
}

void VulkanImageCache::Update()
{
  for(ImageArray& imageArray : mImages.Values())
  {
    size_t i = 0;
    while(i < imageArray.Size())
    {
      CachedImage& cachedImage = imageArray[i];
      ++cachedImage.mTimeSinceUse;
      if(cachedImage.mTimeSinceUse > mInfo.mUsageTimer)
      {
        delete cachedImage.mImage;
        imageArray[i] = imageArray.Back();
        imageArray.PopBack();
        continue;
      }
      ++i;
    }
  }
}

void VulkanImageCache::Clear()
{
  for(ImageArray& imageArray : mImages.Values())
  {
    for(CachedImage& cachedImage : imageArray)
    {
      delete cachedImage.mImage;
    }
  }
  mImages.Clear();
}

VulkanImageCache::ImageCacheKey::ImageCacheKey(const VulkanImageCreationInfo& imageCreationInfo)
{
  mType = imageCreationInfo.mType;
  mFormat = imageCreationInfo.mFormat;
  mWidth = imageCreationInfo.mWidth;
  mHeight = imageCreationInfo.mHeight;
  mDepth = imageCreationInfo.mDepth;
}

size_t VulkanImageCache::ImageCacheKey::Hash() const
{
  Hasher hasher;
  hasher.U32(mType);
  hasher.U32(mFormat);
  hasher.U32(mWidth);
  hasher.U32(mHeight);
  hasher.U32(mDepth);
  return hasher.mHash;
}
