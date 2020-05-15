#include "Precompiled.hpp"

#include "GraphicsEngine.hpp"
#include "GraphicsSpace.hpp"

#include "GraphicsBufferTypes.hpp"
#include "VulkanStructures.hpp"
#include "VulkanInitialization.hpp"
#include "VulkanCommandBuffer.hpp"

void GraphicsEngine::Initialize(const GraphicsEngineInitData& initData)
{
  Zilch::ZilchGraphicsLibrary::InitializeInstance();
  auto zilchGraphicsLibrary = Zilch::ZilchGraphicsLibrary::GetLibrary();

  mTextureManager.Load(initData.mResourcesDir);
  mZilchFragmentFileManager.Load(initData.mResourcesDir);
  mZilchMaterialManager.Load(initData.mResourcesDir);

  // Setup the shader manager with some descriptor types and pointers it needs
  ZilchShaderInitData shaderInitData{initData.mShaderCoreDir, &mZilchFragmentFileManager, &mZilchMaterialManager};
  mZilchShaderManager.AddUniformDescriptor(ZilchTypeId(FrameData));
  mZilchShaderManager.AddUniformDescriptor(ZilchTypeId(CameraData));
  mZilchShaderManager.AddUniformDescriptor(ZilchTypeId(TransformData));
  mZilchShaderManager.Initialize(shaderInitData);
  // Build the fragments from the fragment manager then build the shaders from the materials
  mZilchShaderManager.BuildFragmentsLibrary();
  mZilchShaderManager.BuildShadersLibrary();
  
  mMeshManager.Load(initData.mResourcesDir);
}

void GraphicsEngine::Shutdown()
{
  CleanupSwapChain();
  for(Mesh* mesh : mMeshManager.mMeshMap.Values())
  {
    mRenderer.DestroyMesh(mesh);
  }
  for(Texture* texture : mTextureManager.mTextureMap.Values())
  {
    mRenderer.DestroyTexture(texture);
  }
  for(ZilchShader* zilchShader : mZilchShaderManager.Values())
  {
    mRenderer.DestroyShader(zilchShader);
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
  mSpaces.PushBack(space);
  return space;
}

GraphicsSpace* GraphicsEngine::FindSpace(const String& name)
{
  for(size_t i = 0; i < mSpaces.Size(); ++i)
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
      mSpaces.Erase(it);
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
      mSpaces.Erase(it);
    }
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
  for(Texture* texture : mTextureManager.mTextureMap.Values())
  {
    mRenderer.CreateTexture(texture);
  }
}

void GraphicsEngine::LoadVulkanShaders()
{
  for(ZilchShader* shader : mZilchShaderManager.Values())
  {
    mRenderer.CreateShader(shader);
    mRenderer.CreateShaderMaterial(shader);
  }
}

void GraphicsEngine::LoadVulkanMaterial(ZilchMaterial* zilchMaterial)
{
  ZilchShader* zilchShader = mZilchShaderManager.Find(zilchMaterial->mMaterialName);
 
  mRenderer.UpdateShaderMaterialInstance(zilchShader, zilchMaterial);
}

void GraphicsEngine::LoadVulkanMaterials()
{
  for(ZilchMaterial* material : mZilchMaterialManager.mMaterialMap.Values())
  {
    LoadVulkanMaterial(material);
  }
}

void GraphicsEngine::LoadVulkanMeshes()
{
  for(Mesh* mesh : mMeshManager.mMeshMap.Values())
  {
    mRenderer.CreateMesh(mesh);
  }
}

void GraphicsEngine::PopulateMaterialBuffer()
{
  struct BufferSortData
  {
    uint32_t mBufferId;
    size_t mBufferOffset;
    MaterialProperty* mProperty;
    Zero::ShaderResourceReflectionData* mReflectionData;
  };
  auto sortLambda = [](const BufferSortData& rhs, const BufferSortData& lhs)
  {
    return rhs.mBufferId < lhs.mBufferId;
  };
  Array<BufferSortData> propertiesByBuffer;

  for(auto pair : mZilchMaterialManager.mMaterialMap.All())
  {
    ZilchMaterial* material = pair.second;
    ZilchShader* shader = mZilchShaderManager.Find(material->mMaterialName);
    VulkanShaderMaterial* vulkanShaderMaterial = mRenderer.mUniqueZilchShaderMaterialMap[shader];

    for(MaterialFragment& fragment : material->mFragments)
    {
      Zero::ZilchShaderIRType* fragmentShaderType = mZilchShaderManager.FindFragmentType(fragment.mFragmentName);
      for(MaterialProperty& materialProp : fragment.mProperties)
      {
        Zero::ShaderResourceReflectionData* reflectionData = shader->mResources[ShaderStage::Pixel].mReflection->FindUniformReflectionData(fragmentShaderType, materialProp.mPropertyName);
        if(reflectionData != nullptr)
        {
          BufferSortData sortData{vulkanShaderMaterial->mBufferId, vulkanShaderMaterial->mBufferOffset, &materialProp, reflectionData};
          propertiesByBuffer.PushBack(sortData);
        }
      }
    }
  }

  uint32_t bufferId = static_cast<uint32_t>(-1);
  VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
  byte* byteData = nullptr;
  for(size_t i = 0; i < propertiesByBuffer.Size(); ++i)
  {
    BufferSortData& data = propertiesByBuffer[i];
    if(bufferId != data.mBufferId || byteData == nullptr)
    {
      if(byteData != nullptr)
        mRenderer.UnMapGlobalUniformBufferMemory(MaterialBufferName, bufferId);

      bufferId = data.mBufferId;
      byteData = static_cast<byte*>(mRenderer.MapGlobalUniformBufferMemory(MaterialBufferName, bufferId));
    }

    MaterialProperty* prop = data.mProperty;
    unsigned char* fieldStart = byteData + data.mReflectionData->mOffsetInBytes + data.mBufferOffset;
    // This might be wrong due to stride, have to figure out how to deal with this...
    memcpy(fieldStart, prop->mData.Data(), prop->mData.Size());
  }
}

void GraphicsEngine::CleanupSwapChain()
{
  for(ZilchShader* zilchShader : mZilchShaderManager.Values())
  {
    mRenderer.DestroyShaderMaterial(zilchShader);
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

  for(ZilchMaterial* zilchMaterial: mZilchMaterialManager.mMaterialMap.Values())
  {
    ZilchShader* zilchShader = mZilchShaderManager.Find(zilchMaterial->mMaterialName);
    mRenderer.CreateShaderMaterial(zilchShader);
    mRenderer.UpdateShaderMaterialInstance(zilchShader, zilchMaterial);
  }
}

void GraphicsEngine::WaitIdle()
{
  vkDeviceWaitIdle(mRenderer.mInternal->mDevice);
}
