#pragma once

#include "Shader.hpp"

//-------------------------------------------------------------------MaterialDescriptorEntryType
enum class MaterialDescriptorEntryType
{
  Unknown,
  Byte,
  Int,
  Float,
  Vec2, Vec3, Vec4,
  Mat2, Mat3, Mat4
};

//-------------------------------------------------------------------MaterialDescriptorType
enum class MaterialDescriptorType
{
  Unknown,
  Uniform,
  UniformDynamic,
  SampledImage
};

//-------------------------------------------------------------------MaterialDescriptorElement
struct MaterialDescriptorElement
{
  MaterialDescriptorElement() {}
  MaterialDescriptorElement(MaterialDescriptorEntryType type, size_t offset, size_t size)
    : mElementType(type), mOffset(offset), mSize(size)
  {

  }

  MaterialDescriptorEntryType mElementType = MaterialDescriptorEntryType::Unknown;
  size_t mOffset = 0;
  size_t mSize = 0;
};

//-------------------------------------------------------------------MaterialDescriptorSetLayout
struct MaterialDescriptorSetLayout
{
  size_t mBinding = 0;
  ShaderStageFlags::Enum mStageFlags = ShaderStageFlags::None;
  MaterialDescriptorType mDescriptorType = MaterialDescriptorType::Unknown;
  std::vector<MaterialDescriptorElement> mElements;
  size_t mTotalSize;

  size_t AddElement(MaterialDescriptorEntryType type, size_t offset, size_t size)
  {
    mElements.emplace_back(MaterialDescriptorElement(type, offset, size));
    mTotalSize += size;
    return offset + size;
  }
};

//-------------------------------------------------------------------MaterialBuffer
struct MaterialBuffer
{
  size_t mBinding = 0;
  std::vector<char> mData;
};

//-------------------------------------------------------------------Material
struct Material
{
  Shader* mShader = nullptr;

  std::vector<MaterialBuffer> mBuffers;
  std::vector<MaterialDescriptorSetLayout> mDescriptorLayouts;

  // Remove below here...
  String mVertexShaderName;
  String mPixelShaderName;
};

//-------------------------------------------------------------------MaterialManager
struct MaterialManager
{
public:
  MaterialManager();
  ~MaterialManager();

  void Add(const String& name, Material* material);
  Material* Find(const String& name);
  void Destroy();

  std::unordered_map<String, Material*> mMaterialMap;
};
