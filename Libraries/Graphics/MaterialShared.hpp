#pragma once

#include "ShaderEnumTypes.hpp"
#include "GraphicsStandard.hpp"

typedef unsigned char byte;
class JsonLoader;

//-------------------------------------------------------------------MaterialDescriptorType
enum class MaterialDescriptorType
{
  Unknown,
  Uniform,
  UniformDynamic,
  SampledImage
};

//-------------------------------------------------------------------ShaderMaterialBindingId
struct ShaderMaterialBindingId
{
  enum Enum
  {
    Unknown,
    Global,
    Transforms,
    Image,
    Material
  };
};

//-------------------------------------------------------------------MaterialBuffer
struct MaterialBuffer
{
  size_t mBinding = 0;
  Array<char> mData;
};

//-------------------------------------------------------------------MaterialBuffer
struct MaterialProperty
{
  String mPropertyName;
  ShaderPrimitiveType::Enum mType = ShaderPrimitiveType::Unknown;
  Array<byte> mData;
};

void ReadPropertyValue(JsonLoader& loader, ShaderPrimitiveType::Enum& propType, Array<byte>& data);
void LoadMaterialProperty(JsonLoader& loader, const String& propertyName, MaterialProperty& materialProperty);
void LoadMaterialProperties(JsonLoader& loader, Array<MaterialProperty>& materialProperties);
