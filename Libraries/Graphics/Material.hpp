#pragma once

#include "Shader.hpp"
#include "GraphicsStandard.hpp"

typedef unsigned char byte;

//-------------------------------------------------------------------MaterialDescriptorType
enum class MaterialDescriptorType
{
  Unknown,
  Uniform,
  UniformDynamic,
  SampledImage
};

//-------------------------------------------------------------------MaterialBuffer
struct MaterialBuffer
{
  size_t mBinding = 0;
  Array<char> mData;
};

struct MaterialProperty
{
  String mPropertyName;
  ShaderPrimitiveType::Enum mType = ShaderPrimitiveType::Unknown;
  Array<byte> mData;
};

//-------------------------------------------------------------------Material
struct Material
{
  String mMaterialName;
  String mShaderName;

  Array<MaterialProperty> mProperties;
};

//-------------------------------------------------------------------MaterialManager
struct MaterialManager
{
public:
  MaterialManager();
  ~MaterialManager();

  void Load();
  void LoadFromFile(const String& path);

  void Add(const String& name, Material* material);
  Material* Find(const String& name);
  void Destroy();

  HashMap<String, Material*> mMaterialMap;
};
