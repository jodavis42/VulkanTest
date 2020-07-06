#include "Precompiled.hpp"

#include "GraphicsEngine.hpp"

#include "Resources/ResourceSystem.hpp"
#include "Engine/Composition.hpp"
#include "GraphicsSpace.hpp"

#include "GraphicsBufferTypes.hpp"
#include "RenderQueue.hpp"
#include "VulkanRenderer.hpp"

//-------------------------------------------------------------------GraphicsEngine
ZilchDefineType(GraphicsEngine, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();
}

void GraphicsEngine::Initialize(const CompositionInitializer& initializer)
{
  Zilch::EventConnect(GetOwner(), Events::EngineUpdate, &GraphicsEngine::OnEngineUpdate, this);
}

void GraphicsEngine::InitializeGraphics(const GraphicsEngineInitData& initData)
{
  mInitData = initData;
  Zilch::ZilchGraphicsLibrary::InitializeInstance();
  auto zilchGraphicsLibrary = Zilch::ZilchGraphicsLibrary::GetLibrary();

  mResourceSystem = initData.mResourceSystem;
  mMeshManager = mResourceSystem->FindResourceManager(MeshManager);
  mTextureManager = mResourceSystem->FindResourceManager(TextureManager);
  mZilchFragmentFileManager = mResourceSystem->FindResourceManager(ZilchFragmentFileManager);
  mZilchMaterialManager = mResourceSystem->FindResourceManager(ZilchMaterialManager);
  Zilch::EventConnect(mZilchFragmentFileManager, Events::ResourceLoaded, &GraphicsEngine::OnResourceLoaded, this);
  Zilch::EventConnect(mZilchMaterialManager, Events::ResourceLoaded, &GraphicsEngine::OnResourceLoaded, this);
  Zilch::EventConnect(mZilchFragmentFileManager, Events::ResourceReLoaded, &GraphicsEngine::OnResourceReLoaded, this);
  Zilch::EventConnect(mZilchMaterialManager, Events::ResourceReLoaded, &GraphicsEngine::OnResourceReLoaded, this);

  // Setup the shader manager with some descriptor types and pointers it needs
  ZilchShaderInitData shaderInitData{initData.mShaderCoreDir, mZilchFragmentFileManager, mZilchMaterialManager};
  mZilchShaderManager.AddUniformDescriptor(ZilchTypeId(FrameData));
  mZilchShaderManager.AddUniformDescriptor(ZilchTypeId(CameraData));
  mZilchShaderManager.AddUniformDescriptor(ZilchTypeId(TransformData));
  mZilchShaderManager.Initialize(shaderInitData);
  // Build the fragments from the fragment manager then build the shaders from the materials
  mZilchShaderManager.BuildFragmentsLibrary();
  mZilchShaderManager.BuildShadersLibrary();
}

void GraphicsEngine::Shutdown()
{
  RemoveResourcesFromRenderer();
  mRenderer.CleanupResources();
  mRenderer.Cleanup();
  mRenderer.Shutdown();
  mRenderer.Destroy();
}

void GraphicsEngine::Add(GraphicsSpace* space)
{
  mSpaces.PushBack(space);
}

void GraphicsEngine::Remove(GraphicsSpace* space)
{
  size_t index = mSpaces.FindIndex(space);
  Math::Swap(mSpaces[index], mSpaces[mSpaces.Size() - 1]);
  mSpaces.PopBack();
}

void GraphicsEngine::OnEngineUpdate(Zilch::EventData* e)
{
  Update();
}

void GraphicsEngine::Update()
{
  if(mReloadResources)
    ReloadResources();

  RenderFrameStatus status = mRenderer.BeginFrame();
  if(status == RenderFrameStatus::OutOfDate)
  {
    Reshape();
    return;
  }

  RenderQueue renderQueue;
  for(GraphicsSpace* space : mSpaces)
  {
    space->RenderQueueUpdate(renderQueue);
  }
  mRenderer.DrawRenderQueue(renderQueue);

  status = mRenderer.EndFrame();
  if(status == RenderFrameStatus::OutOfDate || status == RenderFrameStatus::SubOptimal)
    Reshape();
  else if(status != RenderFrameStatus::Success)
  {
    ErrorIf(true, "failed to present swap chain image!");
  }
}

Renderer* GraphicsEngine::GetRenderer()
{
  return &mRenderer;
}

void GraphicsEngine::InitializeRenderer(GraphicsEngineRendererInitData& rendererInitData)
{
  VulkanInitializationData vulkanInitData;
  vulkanInitData.mWidth = rendererInitData.mInitialWidth;
  vulkanInitData.mHeight = rendererInitData.mInitialHeight;
  vulkanInitData.mSurfaceCreationCallback = rendererInitData.mSurfaceCreationCallback;
  mRenderer.Initialize(vulkanInitData);

  UploadResourcesToRenderer();
}

void GraphicsEngine::UploadResourcesToRenderer()
{
  UploadImages();
  UploadShaders();
  UploadMaterials();
  UploadMeshes();
}

void GraphicsEngine::RemoveResourcesFromRenderer()
{
  RemoveMeshes();
  RemoveMaterials();
  RemoveShaders();
  RemoveImages();
}

void GraphicsEngine::UploadImages()
{
  for(Texture* texture : mTextureManager->Resources())
  {
    mRenderer.CreateTexture(texture);
  }
}

void GraphicsEngine::RemoveImages()
{
  for(Texture* texture : mTextureManager->Resources())
  {
    mRenderer.DestroyTexture(texture);
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

void GraphicsEngine::RemoveShaders()
{
  for(ZilchShader* shader : mZilchShaderManager.Values())
  {
    mRenderer.DestroyShader(shader);
    mRenderer.DestroyShaderMaterial(shader);
  }
}

void GraphicsEngine::UploadMaterials()
{
  for(ZilchMaterial* zilchMaterial : mZilchMaterialManager->Resources())
  {
    ZilchShader* zilchShader = mZilchShaderManager.Find(zilchMaterial->mMaterialName);
    if(zilchShader != nullptr)
      mRenderer.UpdateShaderMaterialInstance(zilchShader, zilchMaterial);
  }
}

void GraphicsEngine::RemoveMaterials()
{
}

void GraphicsEngine::UploadMeshes()
{
  for(Mesh* mesh : mMeshManager->Resources())
  {
    mRenderer.CreateMesh(mesh);
  }
}

void GraphicsEngine::RemoveMeshes()
{
  for(Mesh* mesh : mMeshManager->Resources())
  {
    mRenderer.DestroyMesh(mesh);
  }
}

void GraphicsEngine::ReloadResources()
{
  WaitIdle();
  
  RemoveShaders();
  mRenderer.BeginReshape();
  
  mZilchShaderManager.BuildFragmentsLibrary();
  mZilchShaderManager.BuildShadersLibrary();

  mRenderer.EndReshape();

  // This is heavier than needs to happen as a lot of the shader's don't have to be destroyed (modules, etc...). For simplicity do everything right now.
  UploadShaders();
  UploadMaterials();
  PopulateMaterialBuffer();

  mReloadResources = false;
}

void GraphicsEngine::OnResourceLoaded(ResourceLoadEvent* event)
{
  mReloadResources = true;
}

void GraphicsEngine::OnResourceReLoaded(ResourceLoadEvent* event)
{
  mReloadResources = true;
}

void GraphicsEngine::PopulateMaterialBuffer()
{
  MaterialBatchUploadData materialBatchUploadData;
  materialBatchUploadData.mZilchMaterialManager = mZilchMaterialManager;
  materialBatchUploadData.mZilchShaderManager = &mZilchShaderManager;
  for(ZilchMaterial* zilchMaterial : mZilchMaterialManager->Resources())
  {
    MaterialBatchUploadData::MaterialData& materialData = materialBatchUploadData.mMaterials.PushBack();
    materialData.mZilchMaterial = zilchMaterial;
    materialData.mZilchShader = mZilchShaderManager.Find(zilchMaterial->mMaterialName);
  }
  mRenderer.UploadShaderMaterialInstances(materialBatchUploadData);
}

void GraphicsEngine::Reshape()
{
  size_t width, height;
  mWindowSizeQueryFn(width, height);

  WaitIdle();

  RemoveShaders();
  mRenderer.BeginReshape();
  mRenderer.Reshape(width, height, width / (float)height);
  mRenderer.EndReshape();

  // This is heavier than needs to happen as a lot of the shader's don't have to be destroyed (modules, etc...). For simplicity do everything right now.
  UploadShaders();
  UploadMaterials();
  PopulateMaterialBuffer();
}

void GraphicsEngine::WaitIdle()
{
  mRenderer.WaitForIdle();
}
