#include "pch.h"

#include "Shader.hpp"
#include "File.hpp"

ShaderManager::ShaderManager()
{

}

ShaderManager::~ShaderManager()
{
  Destroy();
}

void ShaderManager::Load()
{
  
}

void ShaderManager::LoadShader(const String& name, const ShaderLoadData& shaderData)
{
  Shader* shader = new Shader();

  for(size_t i = 0; i < ShaderStageCount; ++i)
    readFile(shaderData.mShaderCodePaths[i], shader->mShaderByteCode[i]);

  mShaderMap[name] = shader;
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
