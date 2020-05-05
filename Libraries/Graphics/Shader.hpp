#pragma once

#include <string>
#include <unordered_map>
#include <vector>
using String = std::string;

#include "ShaderEnumTypes.hpp"

constexpr size_t ShaderStageCount = static_cast<size_t>(ShaderStage::Count);

//-------------------------------------------------------------------ShaderResourceField
struct ShaderResourceField
{
  ShaderPrimitiveType::Enum mPrimitiveType = ShaderPrimitiveType::Unknown;
  String mName;
  String mType;
  size_t mSizeInBytes = 0;
  size_t mOffset = 0;
  size_t mStride = 0;
};

//-------------------------------------------------------------------ShaderResource
struct ShaderResource
{
  ShaderResourceType::Enum mResourceType = ShaderResourceType::Unknown;
  String mResourceName;
  String mResourceTypeName;
  size_t mBindingId = 0;
  size_t mLocation = 0;
  size_t mDescriptorSet = 0;
  size_t mSizeInBytes = 0;
  std::vector<ShaderResourceField> mFields;
};

//-------------------------------------------------------------------ShaderResources
struct ShaderResources
{
  std::vector<ShaderResource> mUniformBuffers;
  std::vector<ShaderResource> mSampledImages;
  String mEntryPointName;
};

//-------------------------------------------------------------------ShaderLoadData
struct ShaderLoadData
{
  String mShaderCodePaths[ShaderStageCount]{};
};

//-------------------------------------------------------------------Shader
struct Shader
{
public:
  std::vector<char> mShaderByteCode[ShaderStageCount]{};
  ShaderResources mResources[ShaderStageCount]{};
};

//-------------------------------------------------------------------ShaderManager
struct ShaderManager
{
public:
  ShaderManager();
  ~ShaderManager();

  void Load();
  void LoadFromFile(const String& path);
  void LoadShader(const String& name, const ShaderLoadData& shaderData);
  void LoadShaderResources(const std::vector<char>& shaderByteCode, ShaderResources& resources);
  Shader* Find(const String& name);
  void Destroy();

  std::unordered_map<String, Shader*> mShaderMap;
};
