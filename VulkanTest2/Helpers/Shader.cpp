#include "pch.h"

#include "Shader.hpp"

#include <filesystem>
#include "File.hpp"
#include "JsonSerializers.hpp"

#include <vulkan/spirv.h>
#include <SPIRV-Cross/spirv_cross.hpp>
#include <sstream>

ShaderPrimitiveType::Enum LoadPrimitiveType(spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& spirvType)
{
  // Image types are parameterized types. We have to walk all of
  // the sub-parameters to get the final zilch name.
  if(spirvType.basetype == spirv_cross::SPIRType::Image)
  {
    //if(spirVType.image.depth)
    //{
    //  if(spirVType.image.dim == spv::Dim2D)
    //    reflectionData.mTypeName = ZilchTypeId(Zilch::DepthImage2d)->ToString();
    //}
    //else
    //{
    //  if(spirVType.image.dim == spv::Dim2D)
    //    reflectionData.mTypeName = ZilchTypeId(Zilch::Image2d)->ToString();
    //  else if(spirVType.image.dim == spv::DimCube)
    //    reflectionData.mTypeName = ZilchTypeId(Zilch::ImageCube)->ToString();
    //}
  }
  // Sampled images are also parameterized.
  else if(spirvType.basetype == spirv_cross::SPIRType::SampledImage)
  {
    if(spirvType.image.depth)
    {
      if(spirvType.image.dim == spv::Dim2D)
        return ShaderPrimitiveType::SampledImage;
        //reflectionData.mTypeName = ZilchTypeId(Zilch::SampledDepthImage2d)->ToString();
    }
    else
    {
      if(spirvType.image.dim == spv::Dim2D)
        return ShaderPrimitiveType::SampledImage;
        //reflectionData.mTypeName = ZilchTypeId(Zilch::SampledImage2d)->ToString();
      else if(spirvType.image.dim == spv::DimCube)
        return ShaderPrimitiveType::SampledImage;
        //reflectionData.mTypeName = ZilchTypeId(Zilch::SampledImageCube)->ToString();
    }
  }
  else if(spirvType.basetype == spirv_cross::SPIRType::Sampler)
  {
    //reflectionData.mTypeName = ZilchTypeId(Zilch::Sampler)->ToString();
  }
  // Otherwise, assume this is either a primitive type or a struct
  else
  {
    std::stringstream stream;
    // Get the base type.
    if(spirvType.basetype == spirv_cross::SPIRType::Float)
      stream << "Float";
    else if(spirvType.basetype == spirv_cross::SPIRType::Int)
      stream << "Int";
    //else if(spirVType.basetype == spirv_cross::SPIRType::Boolean)
    //  reflectionData.mTypeName = "Boolean";
    else
      stream << compiler.get_name(spirvType.self).c_str();

    // Append dimensionality
    if(spirvType.columns > 1)
      stream << spirvType.columns << "x" << spirvType.vecsize;
    else if(spirvType.vecsize > 1)
      stream << spirvType.vecsize;

    String fullName = stream.str();
    return ShaderPrimitiveType::FromString(fullName);
  }
  return ShaderPrimitiveType::Unknown;
}

//-------------------------------------------------------------------ShaderManager
ShaderManager::ShaderManager()
{

}

ShaderManager::~ShaderManager()
{
  Destroy();
}

void ShaderManager::Load()
{
  LoadAllFilesOfExtension(*this, "data", ".shader");
}

void ShaderManager::LoadFromFile(const String& path)
{
  JsonLoader loader;
  loader.LoadFromFile(path);

  String name;
  ShaderLoadData shaderData;
  LoadPrimitive(loader, "Name", name);
  LoadPrimitive(loader, "VertexShaderPath", shaderData.mShaderCodePaths[ShaderStage::Vertex]);
  LoadPrimitive(loader, "PixelShaderPath", shaderData.mShaderCodePaths[ShaderStage::Pixel]);
  LoadShader(name, shaderData);
}

void ShaderManager::LoadShader(const String& name, const ShaderLoadData& shaderData)
{
  Shader* shader = new Shader();

  for(size_t i = 0; i < ShaderStageCount; ++i)
    readFile(shaderData.mShaderCodePaths[i], shader->mShaderByteCode[i]);

  for(size_t i = ShaderStage::Begin; i < ShaderStage::End; ++i)
    LoadShaderResources(shader->mShaderByteCode[i], shader->mResources[i]);

  mShaderMap[name] = shader;
}

void ShaderManager::LoadShaderResources(const std::vector<char>& shaderByteCode, ShaderResources& stageResources)
{
  uint32_t* words = (uint32_t*)shaderByteCode.data();
  uint32_t wordCount = static_cast<uint32_t>(shaderByteCode.size()) / 4;
  spirv_cross::Compiler compiler(words, wordCount);
  spirv_cross::ShaderResources resources = compiler.get_shader_resources();

  auto extractFn = [&compiler](spirv_cross::Resource& spirvResource, ShaderResource& shaderResource)
  {
    const spirv_cross::SPIRType& baseType = compiler.get_type(spirvResource.base_type_id);
    shaderResource.mResourceTypeName = compiler.get_name(spirvResource.base_type_id);
    shaderResource.mResourceName = compiler.get_name(spirvResource.id);
    shaderResource.mBindingId = compiler.get_decoration(spirvResource.id, spv::Decoration::DecorationBinding);
    shaderResource.mLocation = compiler.get_decoration(spirvResource.id, spv::Decoration::DecorationLocation);
    shaderResource.mDescriptorSet = compiler.get_decoration(spirvResource.id, spv::Decoration::DecorationDescriptorSet);
    if(baseType.basetype != spirv_cross::SPIRType::SampledImage)
      shaderResource.mSizeInBytes = compiler.get_declared_struct_size(baseType);

    uint32_t memberCount = static_cast<uint32_t>(baseType.member_types.size());
    shaderResource.mFields.resize(memberCount);
    for(uint32_t memberIndex = 0; memberIndex < memberCount; ++memberIndex)
    {
      ShaderResourceField& resourceField = shaderResource.mFields[memberIndex];
      const spirv_cross::SPIRType& spirvMemberType = compiler.get_type(baseType.member_types[memberIndex]);
      resourceField.mPrimitiveType = LoadPrimitiveType(compiler, spirvMemberType);
      resourceField.mName = compiler.get_member_name(baseType.self, memberIndex);
      resourceField.mOffset = compiler.type_struct_member_offset(baseType, memberIndex);
      resourceField.mSizeInBytes = compiler.get_declared_struct_member_size(baseType, memberIndex);

      resourceField.mStride = 0;
      // Get array stride
      if(!spirvMemberType.array.empty())
        resourceField.mStride = compiler.type_struct_member_array_stride(baseType, memberIndex);
      // Get matrix stride
      if(spirvMemberType.columns > 1)
        resourceField.mStride = compiler.type_struct_member_matrix_stride(baseType, memberIndex);
    }
  };

  stageResources.mUniformBuffers.resize(resources.uniform_buffers.size());
  for(size_t i = 0; i < resources.uniform_buffers.size(); ++i)
  {
    stageResources.mUniformBuffers[i].mResourceType = ShaderResourceType::Uniform;
    extractFn(resources.uniform_buffers[i], stageResources.mUniformBuffers[i]);
  }
  stageResources.mSampledImages.resize(resources.sampled_images.size());
  for(size_t i = 0; i < resources.sampled_images.size(); ++i)
  {
    stageResources.mSampledImages[i].mResourceType = ShaderResourceType::SampledImage;
    extractFn(resources.sampled_images[i], stageResources.mSampledImages[i]);
  }
}

Shader* ShaderManager::Find(const String& name)
{
  auto it = mShaderMap.find(name);
  if(it == mShaderMap.end())
    return nullptr;
  return it->second;
}

void ShaderManager::Destroy()
{
  for(auto pair : mShaderMap)
    delete pair.second;
  mShaderMap.clear();
}
