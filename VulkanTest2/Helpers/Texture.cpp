#include "pch.h"

#include "Texture.hpp"

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

void TextureManager::Load()
{
  LoadAllFilesOfExtension(*this, "data", ".texture");
}

void TextureManager::LoadFromFile(const String& path)
{
  JsonLoader loader;
  loader.LoadFromFile(path);

  String textureName;
  String texturePath;

  LoadPrimitive(loader, "Name", textureName);
  LoadPrimitive(loader, "TexturePath", texturePath);
  LoadTexture(textureName, texturePath);
}

void TextureManager::LoadTexture(const String& name, const String& path)
{
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  uint32_t pixelsSize = texWidth * texHeight * 4;

  Texture* texture = new Texture();
  texture->mName = name;
  texture->mSizeX = texWidth;
  texture->mSizeY = texHeight;
  texture->mMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
  texture->mTextureData.resize(pixelsSize);
  texture->mFormat = TextureFormat::SRGB8A8;
  memcpy(texture->mTextureData.data(), pixels, pixelsSize);

  stbi_image_free(pixels);

  mTextureMap[name] = texture;
}

Texture* TextureManager::Find(const String& name)
{
  auto it = mTextureMap.find(name);
  if(it == mTextureMap.end())
    return nullptr;
  return it->second;
}

void TextureManager::Destroy()
{
  for(auto pair : mTextureMap)
    delete pair.second;
  mTextureMap.clear();
}
