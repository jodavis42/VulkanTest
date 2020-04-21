#pragma once

#include "Shader.hpp"

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
  std::vector<char> mData;
};

struct MaterialProperty
{
  String mPropertyName;
  ShaderPrimitiveType::Enum mType = ShaderPrimitiveType::Unknown;
  std::vector<byte> mData;
};

//-------------------------------------------------------------------Material
struct Material
{
  String mShaderName;

  std::vector<MaterialProperty> mProperties;
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

  std::unordered_map<String, Material*> mMaterialMap;
};
