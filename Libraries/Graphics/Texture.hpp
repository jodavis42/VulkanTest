#pragma once

#include "GraphicsStandard.hpp"

#include "ResourceManager.hpp"
#include "TextureFormat.hpp"

class ResourceMetaFile;

//-------------------------------------------------------------------TextureType
enum class TextureType
{
  None, Texture2D, TextureCube
};

//-------------------------------------------------------------------TextureFiltering
enum class TextureFiltering
{
  None, Nearest, Bilinear, Trilinear
};

//-------------------------------------------------------------------TextureAddressing
enum class TextureAddressing
{
  None, Clamp, Repeat, Mirror
};

//-------------------------------------------------------------------Texture
struct Texture : public Resource
{
public:
  ZilchDeclareType(Texture, Zilch::TypeCopyMode::ReferenceType);

  size_t mSizeX = 0;
  size_t mSizeY = 0;
  TextureType mType = TextureType::Texture2D;
  TextureFormat::Enum mFormat = TextureFormat::RGBA32f;
  TextureFiltering mMinFilter = TextureFiltering::Bilinear;
  TextureFiltering mMagFilter = TextureFiltering::Bilinear;
  TextureAddressing mAddressingX = TextureAddressing::Repeat;
  TextureAddressing mAddressingY = TextureAddressing::Repeat;
  size_t mMipLevels = 1;
  Array<float> mTextureData;
};

//-------------------------------------------------------------------TextureManager
struct TextureManager : public ResourceManagerTyped<Texture>
{
public:
  ZilchDeclareType(TextureManager, Zilch::TypeCopyMode::ReferenceType);

  TextureManager();
  ~TextureManager();

  virtual void GetExtensions(Array<ResourceExtension>& extensions) const override;
  virtual bool OnLoadResource(const ResourceMetaFile& resourceMeta, Texture* texture) override;
  virtual bool OnReLoadResource(const ResourceMetaFile& resourceMeta, Texture* texture) override;
  
  bool LoadTexture(const ResourcePath& path, Texture* texture) const;
};
