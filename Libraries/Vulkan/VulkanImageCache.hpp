#pragma once

#include "VulkanStandard.hpp"
#include "VulkanImage.hpp"

class VulkanRenderer;

//-------------------------------------------------------------------VulkanImageCacheCreationInfo
struct VulkanImageCacheCreationInfo
{
  VkDevice mDevice = VK_NULL_HANDLE;
  VulkanRenderer* mRenderer = nullptr;
  size_t mUsageTimer = 10;
};

//-------------------------------------------------------------------VulkanImageCache
class VulkanImageCache
{
public:
  VulkanImageCache(const VulkanImageCacheCreationInfo& creationInfo);
  ~VulkanImageCache();

  VulkanImage* FindOrCreateImage(const VulkanImageCreationInfo& imageCreationInfo);
  VulkanImage* CreateImage(const VulkanImageCreationInfo& imageCreationInfo);
  void ReleaseImage(VulkanImage* image);

  void Free();

  void Update();
  void Clear();

private:

  VulkanImageCacheCreationInfo mInfo;

  struct ImageCacheKey
  {
    ImageCacheKey() {};
    ImageCacheKey(const VulkanImageCreationInfo& imageCreationInfo);

    bool operator==(const ImageCacheKey& rhs) const = default;
    size_t Hash() const;

    VkImageType mType = VK_IMAGE_TYPE_2D;
    VkFormat mFormat = VK_FORMAT_R8G8B8A8_SRGB;
    uint32_t mWidth = 1;
    uint32_t mHeight = 1;
    uint32_t mDepth = 1;
  };
  struct CachedImage
  {
    VulkanImage* mImage = nullptr;
    size_t mTimeSinceUse = 0;
  };
  using ImageArray = Array<CachedImage>;
  HashMap<ImageCacheKey, ImageArray> mImages;
};
