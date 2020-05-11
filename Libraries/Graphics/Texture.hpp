#pragma once

#include "GraphicsStandard.hpp"

struct FileLoadData;

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
struct Texture
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
  String mName;
  String mFilePath;
  Array<float> mTextureData;
};

//-------------------------------------------------------------------TextureManager
struct TextureManager
{
public:
  TextureManager();
  ~TextureManager();

  void Load(const String& resourcesDir);
  void LoadFromFile(const FileLoadData& loadData);
  void LoadTexture(const String& name, const String& path);
  Texture* Find(const String& name);
  void Destroy();

  HashMap<String, Texture*> mTextureMap;
};
