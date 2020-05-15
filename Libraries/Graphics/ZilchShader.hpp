#pragma once

#include "GraphicsStandard.hpp"

#include "ShaderEnumTypes.hpp"
#include "MaterialShared.hpp"
#include "ZilchShaders/ZilchShadersStandard.hpp"

struct ZilchFragmentFileManager;
struct ZilchMaterialManager;
struct ZilchMaterial;

constexpr size_t ZilchShaderStageCount = static_cast<size_t>(ShaderStage::Count);

namespace Zilch
{
class ErrorEvent;
}

namespace Zero
{
class TranslationErrorEvent;
class ValidationErrorEvent;
class SimpleZilchShaderIRGenerator;
class SimplifiedShaderReflectionData;
}

//-------------------------------------------------------------------ZilchShaderResourceField
struct ZilchShaderResourceField
{
  ShaderPrimitiveType::Enum mPrimitiveType = ShaderPrimitiveType::Unknown;
  String mName;
  String mType;
  size_t mSizeInBytes = 0;
  size_t mOffset = 0;
  size_t mStride = 0;
};

//-------------------------------------------------------------------ZilchShaderResource
struct ZilchShaderResource
{
  ShaderResourceType::Enum mResourceType = ShaderResourceType::Unknown;
  String mResourceName;
  String mResourceTypeName;
  size_t mBindingId = 0;
  size_t mLocation = 0;
  size_t mDescriptorSet = 0;
  size_t mSizeInBytes = 0;
  std::vector<ZilchShaderResourceField> mFields;
};

struct ZilchMaterialBindingDescriptor
{
  String mName;
  u32 mBindingId = 0;
  MaterialDescriptorType mDescriptorType = MaterialDescriptorType::Unknown;
  ShaderMaterialBindingId::Enum mBufferBindingType = ShaderMaterialBindingId::Unknown;
  ShaderStageFlags::Enum mStageFlags = ShaderStageFlags::None;
  String mSampledImageName;
  u32 mSizeInBytes = 0;
  size_t mOffsetInBytes = 0;
};


//-------------------------------------------------------------------ZilchShaderResources
struct ZilchShaderResources
{
  Array<ZilchShaderResource> mUniformBuffers;
  Array<ZilchShaderResource> mSampledImages;
  Zero::SimpleZilchShaderIRGenerator::SimplifiedReflectionRef mReflection;
  String mEntryPointName;
};

//-------------------------------------------------------------------ZilchShaderLoadData
//struct ZilchShaderLoadData
//{
//  String mShaderCodePaths[ZilchShaderStageCount]{};
//};

//-------------------------------------------------------------------ZilchShader
struct ZilchShader
{
public:
  Array<uint32> mShaderByteCode[ZilchShaderStageCount]{};
  ZilchShaderResources mResources[ZilchShaderStageCount]{};
  Array<ZilchMaterialBindingDescriptor> mBindingDescriptors;
  ZilchMaterial* mMaterial = nullptr;
  String mName;
};

struct ZilchShaderInitData
{
  String mShaderCoreDir;
  ZilchFragmentFileManager* mFragmentFileManager = nullptr;
  ZilchMaterialManager* mMaterialManager = nullptr;
};

//-------------------------------------------------------------------ZilchShaderManager
struct ZilchShaderManager
{
public:
  ZilchShaderManager();
  ~ZilchShaderManager();

  void Initialize(ZilchShaderInitData& initData);
  void AddUniformDescriptor(Zilch::BoundType* boundType);

  ZilchShader* Find(const String& name);
  Zero::ZilchShaderIRType* FindFragmentType(const String& fragmentTypeName);
  HashMap<String, ZilchShader*>::valuerange Values();
  void Destroy();

  void BuildFragmentsLibrary();
  void BuildShadersLibrary();

private:
  Zero::ZilchShaderSpirVSettings* CreateZilchShaderSettings(Zero::SpirVNameSettings& nameSettings);
  void ComposeZilchMaterialShader(ZilchMaterial* zilchMaterial);
  void CreateZilchMaterialShader(ZilchMaterial* zilchMaterial);
  void ExtractMaterialDescriptors(ZilchShader* zilchShader);

  static void OnTranslationError(Zero::TranslationErrorEvent* errorEvent, void* self);
  static void OnCompilationError(Zilch::ErrorEvent* e, void* self);
  static void OnValidationError(Zero::ValidationErrorEvent* e, void* self);
  static void OnError(const String& codeLocation, const String& errorMsg, void* self);

  HashMap<String, ZilchShader*> mZilchShaderMap;
  Zero::SimpleZilchShaderIRGenerator* mShaderIRGenerator = nullptr;

  ZilchFragmentFileManager* mFragmentFileManager = nullptr;
  ZilchMaterialManager* mMaterialManager = nullptr;
  Array<Zilch::BoundType*> mUniformDescriptors;
};
