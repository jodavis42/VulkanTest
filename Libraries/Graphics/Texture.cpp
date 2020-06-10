#include "Precompiled.hpp"

#include "Texture.hpp"
#undef Error

#include "Resources/ResourceMetaFile.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

//-------------------------------------------------------------------Texture
ZilchDefineType(Texture, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}

//-------------------------------------------------------------------TextureManager
TextureManager::TextureManager()
{

}

TextureManager::~TextureManager()
{
}

void TextureManager::GetExtensions(Array<ResourceExtension>& extensions) const
{
  extensions.PushBack({"png"});
}

bool TextureManager::OnLoadResource(const ResourceMetaFile& resourceMeta, Texture* texture)
{
  return LoadTexture(resourceMeta.mResourcePath, texture);
}

bool TextureManager::OnReLoadResource(const ResourceMetaFile& resourceMeta, Texture* texture)
{
  return LoadTexture(resourceMeta.mResourcePath, texture);
}

bool TextureManager::LoadTexture(const ResourcePath& path, Texture* texture) const
{
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  if(pixels == nullptr)
  {
    Warn("Failed to load image '%s'", path.c_str());
    return false;
  }
  
  uint32_t pixelsSize = texWidth * texHeight * 4;
  
  texture->mSizeX = texWidth;
  texture->mSizeY = texHeight;
  texture->mMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
  texture->mFormat = TextureFormat::SRGB8A8;
  texture->mTextureData.Resize(pixelsSize);
  memcpy(texture->mTextureData.Data(), pixels, pixelsSize);

  stbi_image_free(pixels);
  return true;
}
