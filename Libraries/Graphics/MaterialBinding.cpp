#include "Precompiled.hpp"

#include "MaterialBinding.hpp"

//-------------------------------------------------------------------UniqueShaderMaterial
void UniqueShaderMaterial::Initialize(const Shader* shader, MaterialDescriptorType undefinedDescriptorTypes, ShaderMaterialBindingId::Enum undefinedBindingId)
{
  mShader = shader;
  mUndefinedDescriptorTypes = undefinedDescriptorTypes;
  mUndefinedBindingId = undefinedBindingId;
}

void UniqueShaderMaterial::AddBinding(const String& name, MaterialDescriptorType descriptorType, ShaderMaterialBindingId::Enum bindingId)
{
  ShaderResourceBinding& binding = mPredefinedBindings[name];
  binding.mBindingName = name;
  binding.mDescriptorType = descriptorType;
  binding.mMaterialBindingId = bindingId;
}

void UniqueShaderMaterial::CompileBindings()
{
  HashMap<String, const ShaderResource*> shaderResourceNameMap;

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
      if(mFieldNameMap.ContainsKey(shaderBinding->mBindingName))
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
    ShaderResourceBinding* shaderResourceBinding = mPredefinedBindings.FindPointer(bindingName);
    if(shaderResourceBinding != nullptr)
    {
      shaderBinding->mDescriptorType = shaderResourceBinding->mDescriptorType;
      shaderBinding->mMaterialBindingId = shaderResourceBinding->mMaterialBindingId;
    }
    else
    {
      shaderBinding->mDescriptorType = mUndefinedDescriptorTypes;
      shaderBinding->mMaterialBindingId = mUndefinedBindingId;
    }

    // Hookup all field
    size_t fieldCount = shaderResource.mFields.Size();
    for(size_t i = 0; i < fieldCount; ++i)
    {
      const ShaderResourceField& fieldResource = shaderResource.mFields[i];
      ShaderFieldBinding& fieldBinding = shaderBinding->mFieldBindings[fieldResource.mName];
      fieldBinding.mOwningBinding = shaderBinding;
      fieldBinding.mShaderField = &fieldResource;

      // Cannot currently support duplicate field names
      if(mFieldNameMap.ContainsKey(fieldResource.mName))
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

//-------------------------------------------------------------------ShaderMaterialInstance
void ShaderMaterialInstance::CompileBindings(const UniqueShaderMaterial& uniqueShaderMaterial, const Material& material)
{
  mUniqueShaderMaterial = &uniqueShaderMaterial;
  mMaterial = &material;

  for(const MaterialProperty& materialProp : material.mProperties)
  {
    ShaderFieldBinding* fieldBinding = mUniqueShaderMaterial->mFieldNameMap.FindValue(materialProp.mPropertyName, nullptr);
    if(fieldBinding == nullptr)
      continue;

    fieldBinding->mMaterialProperty = &materialProp;
    mMaterialNameMap[materialProp.mPropertyName] = fieldBinding;
  }
}
