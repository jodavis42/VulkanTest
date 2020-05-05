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
  HashMap<String, ShaderFieldBinding> mFieldBindings;
};

//-------------------------------------------------------------------UniqueShaderMaterial
/// Lots of shaders can map to the same unique permutation (same uniforms, constants, images, etc...).
/// In particular, this is true as there can be lots of different materials (different values or instances) that use the same shader.
/// This information is the unique mapping for how any of those make it into a shader pipeline.
struct UniqueShaderMaterial
{
  void Initialize(const Shader* shader, MaterialDescriptorType undefinedDescriptorTypes, ShaderMaterialBindingId::Enum undefinedBindingId);

  void AddBinding(const String& name, MaterialDescriptorType descriptorType, ShaderMaterialBindingId::Enum bindingId);
  void CompileBindings();

  const Shader* mShader = nullptr;
  MaterialDescriptorType mUndefinedDescriptorTypes = MaterialDescriptorType::Unknown;
  ShaderMaterialBindingId::Enum mUndefinedBindingId = ShaderMaterialBindingId::Unknown;
  HashMap<String, ShaderResourceBinding> mPredefinedBindings;
  HashMap<String, ShaderResourceBinding*> mBindings;
  HashMap<String, ShaderFieldBinding*> mFieldNameMap;
};

//-------------------------------------------------------------------ShaderMaterialInstance
/// The mappings for a unique instance of a material. This contains information for how an
/// individual material (and its properties) are mapped to the unique shader material so
/// that the buffers, constants, images, etc... can be filled out easily.
struct ShaderMaterialInstance
{
  void CompileBindings(const UniqueShaderMaterial& uniqueShaderBinding, const Material& material);

  const UniqueShaderMaterial* mUniqueShaderMaterial = nullptr;
  const Material* mMaterial = nullptr;
  HashMap<String, ShaderFieldBinding*> mMaterialNameMap;
};
