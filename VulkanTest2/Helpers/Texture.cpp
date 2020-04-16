#include "pch.h"

#include "Texture.hpp"


#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
const std::string TEXTURE_PATH = "textures/chalet.jpg";

TextureManager::TextureManager()
{

}

TextureManager::~TextureManager()
{
  Destroy();
}

void TextureManager::Load()
{
  LoadTexture("Test", TEXTURE_PATH);
}

void TextureManager::LoadTexture(const String& name, const String& path)
{
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  uint32_t pixelsSize = texWidth * texHeight * 4;

  Texture* texture = new Texture();
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
