#pragma once

#include "GraphicsStandard.hpp"

#include "ResourceManager.hpp"

class ResourceMetaFile;

//-------------------------------------------------------------------TextureFormat
enum class  TextureFormat
{
  None,
  R8, RG8, RGB8, RGBA8,                  // byte
  R16, RG16, RGB16, RGBA16,              // short
  R16f, RG16f, RGB16f, RGBA16f,          // half float
  R32f, RG32f, RGB32f, RGBA32f,          // float
  SRGB8, SRGB8A8,                        // gamma
  Depth16, Depth24, Depth32, Depth32f,   // depth
  Depth24Stencil8, Depth32fStencil8Pad24 // depth-stencil
};

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
  size_t mSizeX = 0;
  size_t mSizeY = 0;
  TextureType mType = TextureType::Texture2D;
  TextureFormat mFormat = TextureFormat::RGBA32f;
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
  TextureManager();
  ~TextureManager();

  virtual void GetExtensions(Array<ResourceExtension>& extensions) const override;
  virtual bool OnLoadResource(const ResourceMetaFile& resourceMeta, Texture* texture) override;
  virtual bool OnReLoadResource(const ResourceMetaFile& resourceMeta, Texture* texture) override;
  
  bool LoadTexture(const ResourcePath& path, Texture* texture) const;
};
