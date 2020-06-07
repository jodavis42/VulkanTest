#include "Precompiled.hpp"

#include "Application.hpp"
#include "Utilities/JsonSerializers.hpp"
#include "Resources/ResourceSystem.hpp"
#include "Resources/ResourceLibrary.hpp"
#include "Resources/ResourceLibraryGraph.hpp"
#include "Engine/ArchetypeManager.hpp"
#include "Engine/LevelManager.hpp"
#include "Engine/CompositionInitializer.hpp"
#include "Engine/Composition.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Transform.hpp"
#include "Engine/Space.hpp"
#include "Engine/TimeSpace.hpp"
#include "Graphics/GraphicsEngine.hpp"
#include "Graphics/GraphicsSpace.hpp"
#include "ZilchScript/ZilchScriptManager.hpp"
#include "ZilchScript/ZilchScriptLibrary.hpp"
#include "EngineSerialization.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

VulkanStatus SurfaceCreationCallback(VkInstance instance, void* userData, VkSurfaceKHR& outSurface)
{
  auto application = reinterpret_cast<Application*>(userData);
  auto result = glfwCreateWindowSurface(instance, application->GetWindow(), nullptr, &outSurface);

  VulkanStatus status;
  if(result != VK_SUCCESS)
    status.MarkFailed("failed to create window surface!");
  return status;
}

Application::Application(ApplicationConfig* config)
  : mConfig(config)
  , mZilchScriptLibraryManager(&mResourceSystem)
{
}

void Application::Run()
{
  Initialize();
  MainLoop();
  Shutdown();
}

GLFWwindow* Application::GetWindow()
{
  return mWindow;
}

void Application::Initialize()
{
  LoadConfiguration();
  InitializeResourceSystem();
  BuildZilchScripts();
  BuildEngine();
  BuildSpace();

  mLastFrameTime = std::chrono::high_resolution_clock::now();

  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  const int cWidth = 800;
  const int cHeight = 600;
  mWindow = glfwCreateWindow(cWidth, cHeight, "Vulkan", nullptr, nullptr);
  glfwSetWindowUserPointer(mWindow, this);
  glfwSetFramebufferSizeCallback(mWindow, FramebufferResizeCallback);
  glfwSetKeyCallback(mWindow, &Application::KeyCallback);

  int width = 0, height = 0;
  glfwGetFramebufferSize(mWindow, &width, &height);

  GraphicsEngineInitData graphicsInitData;
  graphicsInitData.mResourcesDir = mResourcesDir;
  graphicsInitData.mShaderCoreDir = mShaderCoreDir;
  graphicsInitData.mResourceSystem = &mResourceSystem;
  GraphicsEngine* graphicsEngine = mEngine->Has<GraphicsEngine>();
  graphicsEngine->InitializeGraphics(graphicsInitData);
  graphicsEngine->mWindowSizeQueryFn = [this](size_t& width, size_t& height) {QueryWindowSize(width, height); };

  GraphicsEngineRendererInitData rendererInitData;
  rendererInitData.mInitialWidth = width;
  rendererInitData.mInitialHeight = height;
  rendererInitData.mSurfaceCreationCallback.mCallbackFn = &SurfaceCreationCallback;
  rendererInitData.mSurfaceCreationCallback.mUserData = this;
  graphicsEngine->InitializeRenderer(rendererInitData);

  graphicsEngine->PopulateMaterialBuffer();
  LoadLevel("Level");
}

void Application::Shutdown()
{
  GraphicsEngine* graphicsEngine = mEngine->Has<GraphicsEngine>();
  graphicsEngine->Shutdown();
  glfwDestroyWindow(mWindow);
  glfwTerminate();
}

void Application::LoadConfiguration()
{
  Zilch::JsonReader jsonReader;
  Zilch::CompilationErrors errors;
  Zilch::JsonValue* json = jsonReader.ReadIntoTreeFromFile(errors, "BuildConfig.data", nullptr);
  mShaderCoreDir = json->GetMember("ShaderCoreDir")->AsString();
  mResourcesDir = json->GetMember("ResourcesDir")->AsString();
}

void Application::InitializeResourceSystem()
{
  mResourceSystem.RegisterResourceManager(Level, LevelManager, new LevelManager());
  mResourceSystem.RegisterResourceManager(ArchetypeManager, ArchetypeManager, new ArchetypeManager());
  mResourceSystem.RegisterResourceManager(ZilchScript, ZilchScriptManager, new ZilchScriptManager());
  mResourceSystem.RegisterResourceManager(ZilchFragmentFile, ZilchFragmentFileManager, new ZilchFragmentFileManager());
  mResourceSystem.RegisterResourceManager(Texture, TextureManager, new TextureManager());
  mResourceSystem.RegisterResourceManager(Mesh, MeshManager, new MeshManager());
  mResourceSystem.RegisterResourceManager(ZilchMaterial, ZilchMaterialManager, new ZilchMaterialManager());
  mResourceSystem.LoadLibrary("BasicProject", Zero::FilePath::Combine(mResourcesDir, "BasicProject"));
}

void Application::BuildZilchScripts()
{
  mZilchScriptLibraryManager.SetNativeDependencies(mConfig->mNativeModule);
  mZilchScriptLibraryManager.BuildLibraries();
  ZilchScriptModule* zilchScriptModule = mZilchScriptLibraryManager.GetModule();
  Zilch::ExecutableState::CallingState = zilchScriptModule->mModule.Link();
}

void Application::BuildEngine()
{
  ArchetypeManager* archetypeManager = mResourceSystem.FindResourceManager(ArchetypeManager);
  Archetype* engineArchetype = archetypeManager->FindResource(ResourceName{"Engine"});
  mEngine = ZilchAllocate(Engine);

  if(engineArchetype != nullptr)
    LoadComposition(engineArchetype->mPath, mEngine);

  if(mEngine->Has<GraphicsEngine>() == nullptr)
    mEngine->AddComponent(ZilchAllocate(GraphicsEngine));
  mEngine->Initialize(CompositionInitializer());
}

void Application::BuildSpace()
{
  ArchetypeManager* archetypeManager = mResourceSystem.FindResourceManager(ArchetypeManager);
  Archetype* spaceArchetype = archetypeManager->FindResource(ResourceName{"Space"});
  mSpace = ZilchAllocate(Space);

  if(spaceArchetype != nullptr)
    LoadComposition(spaceArchetype->mPath, mSpace);

  if(mSpace->Has<TimeSpace>() == nullptr)
    mSpace->AddComponent(ZilchAllocate(TimeSpace));
  if(mSpace->Has<GraphicsSpace>() == nullptr)
    mSpace->AddComponent(ZilchAllocate(GraphicsSpace));
  mEngine->Add(mSpace);
  mSpace->Initialize(CompositionInitializer());
}

void Application::LoadLevel(const String& levelName)
{
  ZilchScriptModule* module = GetActiveModule();
  LevelManager* levelManager = mResourceSystem.FindResourceManager(LevelManager);
  ArchetypeManager* archetypeManager = mResourceSystem.FindResourceManager(ArchetypeManager);
  Level* level = levelManager->FindResource(ResourceName{levelName});
  ReturnIf(level == nullptr, , "Failed to find level '%s'", levelName.c_str());

  ::LoadLevel(module, level, mSpace);
  mSpace->InitializeCompositions(CompositionInitializer());
}

bool Application::LoadComposition(const String& path, Composition* composition)
{
  ZilchScriptModule* module = GetActiveModule();
  return ::LoadComposition(module, path, composition);
}

bool Application::LoadComposition(JsonLoader& loader, Composition* composition)
{
  ZilchScriptModule* module = GetActiveModule();
  return ::LoadComposition(module, loader, composition);
}

void Application::ReloadResources()
{
  mResourceSystem.ReloadLibraries();
}

void Application::MainLoop()
{
  while(!glfwWindowShouldClose(mWindow))
  {
    glfwPollEvents();
    ProcessFrame();
  }

  GraphicsEngine* graphicsEngine = mEngine->Has<GraphicsEngine>();
  graphicsEngine->WaitIdle();
}

void Application::ProcessFrame()
{
  constexpr float targetFramerate = 1.0f / 60.0f;
  float dt = 0.0f;
  while(true)
  {
    auto currentTime = std::chrono::high_resolution_clock::now();
    dt = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - mLastFrameTime).count();
    if(dt >= targetFramerate)
    {
      mLastFrameTime = currentTime;
      break;
    }
  }
    
  TimeSpace* timeSpace = mSpace->Has<TimeSpace>();
  if(timeSpace != nullptr)
    timeSpace->Update(dt);
    
  DrawFrame();

  for(Space* space : mEngine->mSpaces)
    space->DestroyQueuedCompositions();
  mEngine->DestroyQueuedCompositions();
}

void Application::DrawFrame()
{
  Zilch::HandleOf<Zilch::EventData> toSend = ZilchAllocate(Zilch::EventData);
  toSend->EventName = Events::EngineUpdate;
  Zilch::EventSend(mEngine, toSend->EventName, toSend);
}

ZilchScriptModule* Application::GetActiveModule()
{
  return mZilchScriptLibraryManager.GetModule();
}

void Application::QueryWindowSize(size_t& outWidth, size_t& outHeight)
{
  int width = 0, height = 0;
  while(width == 0 || height == 0)
  {
    glfwGetFramebufferSize(mWindow, &width, &height);
    glfwWaitEvents();
  }
  outWidth = width;
  outHeight = height;
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  auto self = (Application*)glfwGetWindowUserPointer(window);
  if(key == GLFW_KEY_R && action == GLFW_PRESS)
    self->ReloadResources();
}

void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
  auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
}
