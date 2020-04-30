#include "pch.h"

#include "GraphicsEngine.hpp"
#include "GraphicsSpace.hpp"

#include "VulkanStructures.hpp"
#include "VulkanInitialization.hpp"
#include "VulkanCommandBuffer.hpp"

void GraphicsEngine::Initialize()
{
  mTextureManager.Load();
  mShaderManager.Load();
  mMaterialManager.Load();
  mMeshManager.Load();
  LoadShadersAndMaterials();
}

void GraphicsEngine::Shutdown()
{
  CleanupSwapChain();
  for(auto pair : mMeshManager.mMeshMap)
  {
    Mesh* mesh = pair.second;
    mRenderer.DestroyMesh(mesh);
  }
  for(auto pair : mShaderManager.mShaderMap)
  {
    Shader* shader = pair.second;
    mRenderer.DestroyShader(shader);
  }
  //for(auto pair : mMaterialManager.mMaterialMap)
  //{
  //  Material* material = pair.second;
  //  mRenderer.DestroyShaderMaterial(shader);
  //}
  for(auto pair : mTextureManager.mTextureMap)
  {
    Texture* texture = pair.second;
    mRenderer.DestroyTexture(texture);
  }
  mRenderer.CleanupResources();
  mRenderer.Shutdown();

  if(enableValidationLayers)
    DestroyDebugUtilsMessengerEXT(mRenderer.mInternal->mInstance, mRenderer.mInternal->mDebugMessenger, nullptr);

  vkDestroyCommandPool(mRenderer.mInternal->mDevice, mRenderer.mInternal->mCommandPool, nullptr);

  vkDestroyDevice(mRenderer.mInternal->mDevice, nullptr);
  vkDestroySurfaceKHR(mRenderer.mInternal->mInstance, mRenderer.mInternal->mSurface, nullptr);
  vkDestroyInstance(mRenderer.mInternal->mInstance, nullptr);
}

GraphicsSpace* GraphicsEngine::CreateSpace(const String& name)
{
  GraphicsSpace* space = new GraphicsSpace();
  space->mName = name;
  space->mEngine = this;
  mSpaces.push_back(space);
  return space;
}

GraphicsSpace* GraphicsEngine::FindSpace(const String& name)
{
  for(size_t i = 0; i < mSpaces.size(); ++i)
  {
    if(mSpaces[i]->mName == name)
      return mSpaces[i];
  }
  return nullptr;
}

void GraphicsEngine::DestroySpace(const String& name)
{
  for(auto it = mSpaces.begin(); it != mSpaces.end(); ++it)
  {
    GraphicsSpace* space = *it;
    if(space->mName == name)
    {
      delete space;
      mSpaces.erase(it);
    }
  }
}

void GraphicsEngine::DestroySpace(GraphicsSpace* space)
{
  for(auto it = mSpaces.begin(); it != mSpaces.end(); ++it)
  {
    if(*it == space)
    {
      delete space;
      mSpaces.erase(it);
    }
  }
}

void GraphicsEngine::LoadShadersAndMaterials()
{
  // Build the unique shader bindings for each shader
  for(auto pair : mShaderManager.mShaderMap)
  {
    String shaderName = pair.first;
    Shader* shader = pair.second;

    UniqueShaderMaterial& uniqueShaderMaterial = mUniqueShaderMaterials[shaderName];
    uniqueShaderMaterial.AddBinding("PerCameraData", MaterialDescriptorType::Uniform, ShaderMaterialBindingId::Global);
    uniqueShaderMaterial.AddBinding("PerObjectData", MaterialDescriptorType::UniformDynamic, ShaderMaterialBindingId::Global);
    uniqueShaderMaterial.Initialize(shader, MaterialDescriptorType::Uniform, ShaderMaterialBindingId::Material);
    uniqueShaderMaterial.CompileBindings();
  }

  // For each material, bind it to the unique shader
  for(auto pair : mMaterialManager.mMaterialMap)
  {
    String materialName = pair.first;
    Material* material = pair.second;

    Shader* shader = mShaderManager.Find(material->mShaderName);
    UniqueShaderMaterial& uniqueShaderMaterial = mUniqueShaderMaterials[material->mShaderName];
    ShaderMaterialInstance& materialInstance = mShaderMaterialInstances[shader];
    materialInstance.CompileBindings(uniqueShaderMaterial, *material);
  }
}

void GraphicsEngine::InitializeRenderer(GraphicsEngineRendererInitData& rendererInitData)
{
  VulkanInitializationData vulkanInitData;
  vulkanInitData.mWidth = rendererInitData.mInitialWidth;
  vulkanInitData.mHeight = rendererInitData.mInitialHeight;
  vulkanInitData.mSurfaceCreationCallback = rendererInitData.mSurfaceCreationCallback;
  mRenderer.Initialize(vulkanInitData);

  LoadVulkanImages();
  LoadVulkanShaders();
  LoadVulkanMaterials();
  LoadVulkanMeshes();
}

void GraphicsEngine::LoadVulkanImages()
{
  for(auto pair : mTextureManager.mTextureMap)
  {
    mRenderer.CreateTexture(pair.second);
  }
}

void GraphicsEngine::LoadVulkanShaders()
{
  for(auto pair : mShaderManager.mShaderMap)
  {
    Shader* shader = pair.second;
    mRenderer.CreateShader(shader);
    mRenderer.CreateShaderMaterial(&mUniqueShaderMaterials[pair.first]);
  }
}

void GraphicsEngine::LoadVulkanMaterial(Material* material)
{
  Shader* shader = mShaderManager.Find(material->mShaderName);
  ShaderMaterialInstance& shaderMaterialInstance = mShaderMaterialInstances[shader];

  mRenderer.UpdateShaderMaterialInstance(&shaderMaterialInstance);

  uint32_t offset = 0;
  for(auto pair : shaderMaterialInstance.mUniqueShaderMaterial->mBindings)
  {
    ShaderResourceBinding* shaderBinding = pair.second;
    if(shaderBinding->mMaterialBindingId == ShaderMaterialBindingId::Material)
    {
      shaderBinding->mBufferOffset = offset;
      offset += static_cast<uint32_t>(shaderBinding->mBoundResource->mSizeInBytes);
    }
  }
}

void GraphicsEngine::LoadVulkanMaterials()
{
  for(auto pair : mMaterialManager.mMaterialMap)
  {
    LoadVulkanMaterial(pair.second);
  }
}

void GraphicsEngine::LoadVulkanMeshes()
{
  for(auto pair : mMeshManager.mMeshMap)
  {
    Mesh* mesh = pair.second;
    mRenderer.CreateMesh(mesh);
  }
}

void GraphicsEngine::PopulateMaterialBuffer()
{
  struct BufferSortData
  {
    uint32_t mBufferId;
    MaterialProperty* mProperty;
    ShaderFieldBinding* mFieldBinding;
  };
  auto sortLambda = [](const BufferSortData& rhs, const BufferSortData& lhs)
  {
    return rhs.mBufferId < lhs.mBufferId;
  };
  std::vector<BufferSortData> propertiesByBuffer;

  for(auto pair : mMaterialManager.mMaterialMap)
  {
    Material* material = pair.second;
    Shader* shader = mShaderManager.Find(material->mShaderName);
    ShaderMaterialInstance& shaderMaterialInstance = mShaderMaterialInstances[shader];
    VulkanShaderMaterial* vulkanShaderMaterial = mRenderer.mUniqueShaderMaterialMap[shaderMaterialInstance.mUniqueShaderMaterial];

    for(MaterialProperty& materialProp : material->mProperties)
    {
      auto it = shaderMaterialInstance.mMaterialNameMap.find(materialProp.mPropertyName);
      if(it == shaderMaterialInstance.mMaterialNameMap.end())
        continue;

      ShaderFieldBinding* fieldBinding = it->second;
      const ShaderResourceField* fieldResource = fieldBinding->mShaderField;
      if(fieldResource != nullptr)
      {
        BufferSortData sortData{vulkanShaderMaterial->mBufferId, &materialProp, fieldBinding};
        propertiesByBuffer.emplace_back(sortData);
      }
    }
  }

  uint32_t bufferId = static_cast<uint32_t>(-1);
  VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
  byte* byteData = nullptr;
  for(size_t i = 0; i < propertiesByBuffer.size(); ++i)
  {
    BufferSortData& data = propertiesByBuffer[i];
    if(bufferId != data.mBufferId || byteData == nullptr)
    {
      if(byteData != nullptr)
        mRenderer.UnMapUniformBufferMemory(UniformBufferType::Material, bufferId);

      bufferId = data.mBufferId;
      byteData = static_cast<byte*>(mRenderer.MapUniformBufferMemory(UniformBufferType::Material, bufferId));
    }

    MaterialProperty* prop = data.mProperty;
    unsigned char* fieldStart = byteData + data.mFieldBinding->mShaderField->mOffset + data.mFieldBinding->mOwningBinding->mBufferOffset;
    // This might be wrong due to stride, have to figure out how to deal with this...
    memcpy(fieldStart, prop->mData.data(), prop->mData.size());
  }
}

void GraphicsEngine::CleanupSwapChain()
{
  mRenderer.DestroyUniformBuffer(0);

  for(auto pair : mShaderMaterialInstances)
  {
    ShaderMaterialInstance& shaderMaterialInstance = pair.second;
    mRenderer.DestroyShaderMaterial(shaderMaterialInstance.mUniqueShaderMaterial);
  }
  mRenderer.DestroyRenderFramesInternal();
  mRenderer.DestroySwapChainInternal();
  mRenderer.DestroyDepthResourcesInternal();
}

void GraphicsEngine::RecreateSwapChain()
{
  size_t width, height;
  mWindowSizeQueryFn(width, height);

  vkDeviceWaitIdle(mRenderer.mInternal->mDevice);

  CleanupSwapChain();
  mRenderer.Resize(width, height);
  mRenderer.CreateDepthResourcesInternal();
  mRenderer.CreateSwapChainInternal();
  mRenderer.CreateRenderFramesInternal();

  for(auto pair : mShaderMaterialInstances)
  {
    ShaderMaterialInstance& shaderMaterialInstance = pair.second;
    mRenderer.CreateShaderMaterial(shaderMaterialInstance.mUniqueShaderMaterial);
    mRenderer.UpdateShaderMaterialInstance(&shaderMaterialInstance);
  }
}

void GraphicsEngine::WaitIdle()
{
  vkDeviceWaitIdle(mRenderer.mInternal->mDevice);
}