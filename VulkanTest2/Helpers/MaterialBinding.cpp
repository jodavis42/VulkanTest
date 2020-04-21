#include "pch.h"

#pragma optimize("", off)
#include "MaterialBinding.hpp"

//-------------------------------------------------------------------ShaderBinding
void ShaderBinding::Initialize(const Shader* shader, MaterialDescriptorType undefinedDescriptorTypes, ShaderMaterialBindingId::Enum undefinedBindingId)
{
  mShader = shader;
  mUndefinedDescriptorTypes = undefinedDescriptorTypes;
  mUndefinedBindingId = undefinedBindingId;
}

void ShaderBinding::AddBinding(const String& name, MaterialDescriptorType descriptorType, ShaderMaterialBindingId::Enum bindingId)
{
  ShaderResourceBinding& binding = mPredefinedBindings[name];
  binding.mBindingName = name;
  binding.mDescriptorType = descriptorType;
  binding.mMaterialBindingId = bindingId;
}

void ShaderBinding::CompileBindings()
{
  std::unordered_map<String, const ShaderResource*> shaderResourceNameMap;

  auto imageCompileLambda = [this](const ShaderResource& shaderResource)
  {
    ShaderResourceBinding* shaderBinding = new ShaderResourceBinding();
    if(shaderResource.mResourceType == ShaderResourceType::SampledImage)
    {
      shaderBinding->mBindingName = shaderResource.mResourceName;
      shaderBinding->mBoundResource = &shaderResource;
      shaderBinding->mMaterialBindingId = ShaderMaterialBindingId::Image;
      shaderBinding->mDescriptorType = MaterialDescriptorType::SampledImage;

      ShaderFieldBinding& fieldBinding = shaderBinding->mFieldBindings[shaderBinding->mBindingName];
      fieldBinding.mOwningBinding = shaderBinding;

      // Cannot currently support duplicate field names
      if(mFieldNameMap.find(shaderBinding->mBindingName) != mFieldNameMap.end())
        __debugbreak();

      mFieldNameMap[shaderBinding->mBindingName] = &fieldBinding;
    }
    mBindings[shaderBinding->mBindingName] = shaderBinding;
  };

  auto uniformCompileLambda = [this](const ShaderResource& shaderResource)
  {
    ShaderResourceBinding* shaderBinding = new ShaderResourceBinding();

    String bindingName = shaderResource.mResourceTypeName;
    shaderBinding->mBoundResource = &shaderResource;
    shaderBinding->mBindingName = bindingName;

    // See if this is a predefined resource name
    auto it = mPredefinedBindings.find(bindingName);
    if(it != mPredefinedBindings.end())
    {
      shaderBinding->mDescriptorType = it->second.mDescriptorType;
      shaderBinding->mMaterialBindingId = it->second.mMaterialBindingId;
    }
    else
    {
      shaderBinding->mDescriptorType = mUndefinedDescriptorTypes;
      shaderBinding->mMaterialBindingId = mUndefinedBindingId;
    }

    // Hookup all field
    size_t fieldCount = shaderResource.mFields.size();
    for(size_t i = 0; i < fieldCount; ++i)
    {
      const ShaderResourceField& fieldResource = shaderResource.mFields[i];
      ShaderFieldBinding& fieldBinding = shaderBinding->mFieldBindings[fieldResource.mName];
      fieldBinding.mOwningBinding = shaderBinding;
      fieldBinding.mShaderField = &fieldResource;

      // Cannot currently support duplicate field names
      if(mFieldNameMap.find(fieldResource.mName) != mFieldNameMap.end())
        __debugbreak();

      mFieldNameMap[fieldResource.mName] = &fieldBinding;
    }

    mBindings[bindingName] = shaderBinding;
  };
  for(size_t i = ShaderStage::Begin; i < ShaderStage::End; ++i)
  {
    const ShaderResources& shaderResources = mShader->mResources[i];
    for(const ShaderResource& shaderResource : shaderResources.mUniformBuffers)
      uniformCompileLambda(shaderResource);
    for(const ShaderResource& shaderResource : shaderResources.mSampledImages)
      imageCompileLambda(shaderResource);
  }
}

//-------------------------------------------------------------------ShaderMaterialBinding
void ShaderMaterialBinding::CompileBindings(const ShaderBinding& shaderBinding, const Material& material)
{
  mShaderBinding = &shaderBinding;
  mMaterial = &material;

  for(const MaterialProperty& materialProp : material.mProperties)
  {
    auto it = mShaderBinding->mFieldNameMap.find(materialProp.mPropertyName);
    if(it == mShaderBinding->mFieldNameMap.end())
      continue;

    ShaderFieldBinding* fieldBinding = it->second;
    fieldBinding->mMaterialProperty = &materialProp;
    mMaterialNameMap[materialProp.mPropertyName] = fieldBinding;
  }
}
