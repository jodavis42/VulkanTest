#include "Precompiled.hpp"

#include "GraphicsEngine.hpp"
#include "GraphicsSpace.hpp"

#include "GraphicsBufferTypes.hpp"

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
  mRenderer.CleanupResources();
  mRenderer.Shutdown();
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

  UploadImages();
  UploadShaders();
  UploadMaterials();
  UploadMeshes();
}

void GraphicsEngine::UploadImages()
{
  for(Texture* texture : mTextureManager.mTextureMap.Values())
  {
    mRenderer.CreateTexture(texture);
  }
}

void GraphicsEngine::UploadShaders()
{
  for(ZilchShader* shader : mZilchShaderManager.Values())
  {
    mRenderer.CreateShader(shader);
    mRenderer.CreateShaderMaterial(shader);
  }
}

void GraphicsEngine::UploadMaterial(ZilchMaterial* zilchMaterial)
{
  ZilchShader* zilchShader = mZilchShaderManager.Find(zilchMaterial->mMaterialName);
 
  mRenderer.UpdateShaderMaterialInstance(zilchShader, zilchMaterial);
}

void GraphicsEngine::UploadMaterials()
{
  for(ZilchMaterial* material : mZilchMaterialManager.mMaterialMap.Values())
  {
    UploadMaterial(material);
  }
}

void GraphicsEngine::UploadMeshes()
{
  for(Mesh* mesh : mMeshManager.mMeshMap.Values())
  {
    mRenderer.CreateMesh(mesh);
  }
}

void GraphicsEngine::PopulateMaterialBuffer()
{
  MaterialBatchUploadData materialBatchUploadData;
  materialBatchUploadData.mZilchMaterialManager = &mZilchMaterialManager;
  materialBatchUploadData.mZilchShaderManager = &mZilchShaderManager;
  for(ZilchMaterial* zilchMaterial : mZilchMaterialManager.mMaterialMap.Values())
  {
    MaterialBatchUploadData::MaterialData& materialData = materialBatchUploadData.mMaterials.PushBack();
    materialData.mZilchMaterial = zilchMaterial;
    materialData.mZilchShader = mZilchShaderManager.Find(zilchMaterial->mMaterialName);
  }
  mRenderer.UploadShaderMaterialInstances(materialBatchUploadData);
}

void GraphicsEngine::CleanupSwapChain()
{
  for(ZilchMaterial* zilchMaterial : mZilchMaterialManager.mMaterialMap.Values())
  {
    ZilchShader* zilchShader = mZilchShaderManager.Find(zilchMaterial->mMaterialName);
    mRenderer.DestroyShaderMaterial(zilchShader);
    mRenderer.DestroyShader(zilchShader);
  }

  mRenderer.DestroyRenderFramesInternal();
  mRenderer.DestroySwapChainInternal();
  mRenderer.DestroyDepthResourcesInternal();
}

void GraphicsEngine::RecreateSwapChain()
{
  size_t width, height;
  mWindowSizeQueryFn(width, height);

  WaitIdle();

  CleanupSwapChain();
  mRenderer.Reshape(width, height, width / (float)height);
  mRenderer.CreateDepthResourcesInternal();
  mRenderer.CreateSwapChainInternal();
  mRenderer.CreateRenderFramesInternal();

  // This is heavier than needs to happen as a lot of the shader's don't have to be destroyed (modules, etc...). For simplicity do everything right now.
  UploadShaders();
  UploadMaterials();
  PopulateMaterialBuffer();
}

void GraphicsEngine::WaitIdle()
{
  mRenderer.WaitForIdle();
}
