#pragma once

#include "GraphicsStandard.hpp"

#include "ResourceManager.hpp"

class ResourceMetaFile;

//-------------------------------------------------------------------TextureFormat
namespace TextureFormat
{
enum Enum
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
}
