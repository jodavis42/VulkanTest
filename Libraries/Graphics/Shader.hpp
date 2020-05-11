#pragma once

#include "GraphicsStandard.hpp"

#include "ShaderEnumTypes.hpp"

struct FileLoadData;

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
  Array<ShaderResourceField> mFields;
};

//-------------------------------------------------------------------ShaderResources
struct ShaderResources
{
  Array<ShaderResource> mUniformBuffers;
  Array<ShaderResource> mSampledImages;
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
  Array<char> mShaderByteCode[ShaderStageCount]{};
  ShaderResources mResources[ShaderStageCount]{};
};

//-------------------------------------------------------------------ShaderManager
struct ShaderManager
{
public:
  ShaderManager();
  ~ShaderManager();

  void Load(const String& resourcesDir);
  void LoadFromFile(const FileLoadData& loadData);
  void LoadShader(const String& name, const ShaderLoadData& shaderData);
  void LoadShaderResources(const Array<char>& shaderByteCode, ShaderResources& resources);
  Shader* Find(const String& name);
  void Destroy();

  HashMap<String, Shader*> mShaderMap;
};
