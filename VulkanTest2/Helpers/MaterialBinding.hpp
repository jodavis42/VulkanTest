#pragma once

#include "Material.hpp"
#include "Shader.hpp"

struct ShaderResourceBinding;

//-------------------------------------------------------------------ShaderMaterialBindingId
struct ShaderMaterialBindingId
{
  enum Enum
  {
    Unknown,
    Global,
    Image,
    Material
  };
};

//-------------------------------------------------------------------ShaderFieldBinding
struct ShaderFieldBinding
{
  const ShaderResourceField* mShaderField = nullptr;
  const MaterialProperty* mMaterialProperty = nullptr;
  ShaderResourceBinding* mOwningBinding = nullptr;
};

//-------------------------------------------------------------------ShaderResourceBinding
struct ShaderResourceBinding
{
  String mBindingName;
  MaterialDescriptorType mDescriptorType = MaterialDescriptorType::Unknown;
  ShaderMaterialBindingId::Enum mMaterialBindingId = ShaderMaterialBindingId::Unknown;
  uint32_t mBufferOffset = 0;
  const ShaderResource* mBoundResource = nullptr;
  std::unordered_map<String, ShaderFieldBinding> mFieldBindings;
};

//-------------------------------------------------------------------ShaderBinding
struct ShaderBinding
{
  void Initialize(const Shader* shader, MaterialDescriptorType undefinedDescriptorTypes, ShaderMaterialBindingId::Enum undefinedBindingId);

  void AddBinding(const String& name, MaterialDescriptorType descriptorType, ShaderMaterialBindingId::Enum bindingId);
  void CompileBindings();

  const Shader* mShader = nullptr;
  MaterialDescriptorType mUndefinedDescriptorTypes = MaterialDescriptorType::Unknown;
  ShaderMaterialBindingId::Enum mUndefinedBindingId = ShaderMaterialBindingId::Unknown;
  std::unordered_map<String, ShaderResourceBinding> mPredefinedBindings;
  std::unordered_map<String, ShaderResourceBinding*> mBindings;
  std::unordered_map<String, ShaderFieldBinding*> mFieldNameMap;
};

//-------------------------------------------------------------------ShaderMaterialBinding
struct ShaderMaterialBinding
{
  void CompileBindings(const ShaderBinding& shaderBinding, const Material& material);

  const ShaderBinding* mShaderBinding = nullptr;
  const Material* mMaterial = nullptr;
  std::unordered_map<String, ShaderFieldBinding*> mMaterialNameMap;
};
