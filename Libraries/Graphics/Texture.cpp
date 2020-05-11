#include "Precompiled.hpp"

#include "Texture.hpp"
#undef Error

#include <filesystem>
#include <algorithm>
#include "JsonSerializers.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

//-------------------------------------------------------------------TextureManager
TextureManager::TextureManager()
{

}

TextureManager::~TextureManager()
{
  Destroy();
}

void TextureManager::Load(const String& resourcesDir)
{
  FileSearchData searchData = {resourcesDir, Zero::FilePath::Combine(resourcesDir, "data")};
  LoadAllFilesOfExtension(*this, searchData, ".texture");
}

void TextureManager::LoadFromFile(const FileLoadData& loadData)
{
  JsonLoader loader;
  loader.LoadFromFile(loadData.mFilePath);

  String textureName;
  String texturePath;

  LoadPrimitive(loader, "Name", textureName);
  LoadPrimitive(loader, "TexturePath", texturePath);

  String fullTexturePath = Zero::FilePath::Combine(loadData.mRootResourcesDir, texturePath);
  LoadTexture(textureName, fullTexturePath);
}

void TextureManager::LoadTexture(const String& name, const String& path)
{
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  ErrorIf(pixels == nullptr, "Failed to load image");
  uint32_t pixelsSize = texWidth * texHeight * 4;

  Texture* texture = new Texture();
  texture->mName = name;
  texture->mFilePath = path;
  texture->mSizeX = texWidth;
  texture->mSizeY = texHeight;
  texture->mMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
  texture->mFormat = TextureFormat::SRGB8A8;
  texture->mTextureData.Resize(pixelsSize);
  memcpy(texture->mTextureData.Data(), pixels, pixelsSize);

  stbi_image_free(pixels);

  mTextureMap[name] = texture;
}

Texture* TextureManager::Find(const String& name)
{
  return mTextureMap.FindValue(name, nullptr);
}

void TextureManager::Destroy()
{
  for(Texture* texture : mTextureMap.Values())
    delete texture;
  mTextureMap.Clear();
}
