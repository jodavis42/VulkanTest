#include "Precompiled.hpp"

#include "Utilities/File.hpp"
#include "Utilities/JsonSerializers.hpp"
#include "Resources/ResourceZilchStaticLibrary.hpp"
#include "Resources/ResourceSystem.hpp"
#include "Resources/ResourceLibrary.hpp"
#include "Resources/ResourceLibraryGraph.hpp"
#include "Engine/EngineZilchStaticLibrary.hpp"
#include "Engine/ArchetypeManager.hpp"
#include "Engine/LevelManager.hpp"
#include "Engine/CompositionInitializer.hpp"
#include "Engine/Composition.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Transform.hpp"
#include "Engine/Space.hpp"
#include "Engine/TimeSpace.hpp"
#include "Graphics/GraphicsZilchStaticLibrary.hpp"
#include "Graphics/GraphicsEngine.hpp"
#include "Graphics/GraphicsSpace.hpp"
#include "ZilchScript/ZilchScriptManager.hpp"
#include "ZilchScript/ZilchScriptZilchStaticLibrary.hpp"
#include "ZilchScript/ZilchScriptLibrary.hpp"
#include "EngineSerialization.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <chrono>
#include <iostream>

struct Setup
{
  Zilch::ZilchSetup* mZilchSetup = nullptr;
  Zilch::Module* mNativeModule = nullptr;
  Setup()
  {
    Zilch::ZilchSetup* zilchSetup = new Zilch::ZilchSetup();

    mNativeModule = new Zilch::Module();
    ResourceStaticLibrary::InitializeInstance();
    mNativeModule->PushBack(ResourceStaticLibrary::GetInstance().GetLibrary());
    EngineStaticLibrary::InitializeInstance();
    mNativeModule->PushBack(EngineStaticLibrary::GetInstance().GetLibrary());
    GraphicsStaticLibrary::InitializeInstance();
    mNativeModule->PushBack(GraphicsStaticLibrary::GetInstance().GetLibrary());
    ZilchScriptStaticLibrary::InitializeInstance();
    mNativeModule->PushBack(ZilchScriptStaticLibrary::GetInstance().GetLibrary());
    Zilch::ExecutableState::CallingState = mNativeModule->Link();
  }

  ~Setup()
  {

  }
};

class HelloTriangleApplication {
public:
  HelloTriangleApplication(Setup* setup) : mSetup(setup), mZilchScriptLibraryManager(&mResourceSystem)
  {
  }

  void run()
  {
    Initialize();
    mainLoop();
    Shutdown();
  }

private:
  Setup* mSetup = nullptr;
  String mResourcesDir;
  String mShaderCoreDir;
  ResourceSystem mResourceSystem;
  ZilchScriptLibraryManager mZilchScriptLibraryManager;
  Zilch::HandleOf<Engine> mEngine;
  Zilch::HandleOf<Space> mSpace;

  static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
  {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
  }

  static VulkanStatus SurfaceCreationCallback(VkInstance instance, void* userData, VkSurfaceKHR& outSurface)
  {
    auto self = reinterpret_cast<HelloTriangleApplication*>(userData);
    auto result = glfwCreateWindowSurface(instance, self->mWindow, nullptr, &outSurface);

    VulkanStatus status;
    if(result != VK_SUCCESS)
      status.MarkFailed("failed to create window surface!");
    return status;
  }

  void QueryWindowSize(size_t& outWidth, size_t& outHeight)
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

  void LoadConfiguration()
  {
    Zilch::JsonReader jsonReader;
    Zilch::CompilationErrors errors;
    Zilch::JsonValue* json = jsonReader.ReadIntoTreeFromFile(errors, "BuildConfig.data", nullptr);
    mShaderCoreDir = json->GetMember("ShaderCoreDir")->AsString();
    mResourcesDir = json->GetMember("ResourcesDir")->AsString();
  }

  void InitializeResourceSystem()
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

  void BuildZilchScripts()
  {
    mZilchScriptLibraryManager.SetNativeDependencies(mSetup->mNativeModule);
    mZilchScriptLibraryManager.BuildLibraries();
    ZilchScriptModule* zilchScriptModule = mZilchScriptLibraryManager.GetModule();
    Zilch::ExecutableState::CallingState = zilchScriptModule->mModule.Link();
  }

  void BuildEngine()
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

  void BuildSpace()
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

  void Initialize()
  {
    LoadConfiguration();
    InitializeResourceSystem();
    BuildZilchScripts();
    BuildEngine();
    BuildSpace();

    mTotalFrameTime = 0;
    mLastFrameTime = std::chrono::high_resolution_clock::now();

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const int cWidth = 800;
    const int cHeight = 600;
    mWindow = glfwCreateWindow(cWidth, cHeight, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, FramebufferResizeCallback);
    glfwSetKeyCallback(mWindow, &HelloTriangleApplication::KeyCallback);
    
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
    rendererInitData.mSurfaceCreationCallback.mCallbackFn = &HelloTriangleApplication::SurfaceCreationCallback;
    rendererInitData.mSurfaceCreationCallback.mUserData = this;
    graphicsEngine->InitializeRenderer(rendererInitData);

    graphicsEngine->PopulateMaterialBuffer();
    LoadLevel("Level");
  }

  ZilchScriptModule* GetActiveModule()
  {
    return mZilchScriptLibraryManager.GetModule();
  }

  void LoadLevel(const String& levelName)
  {
    ZilchScriptModule* module = GetActiveModule();
    LevelManager* levelManager = mResourceSystem.FindResourceManager(LevelManager);
    ArchetypeManager* archetypeManager = mResourceSystem.FindResourceManager(ArchetypeManager);
    Level* level = levelManager->FindResource(ResourceName{levelName});
    ReturnIf(level == nullptr, , "Failed to find level '%s'", levelName.c_str());

    ::LoadLevel(module, level, mSpace);
    mSpace->InitializeCompositions(CompositionInitializer());
  }

  bool LoadComposition(const String& path, Composition* composition)
  {
    ZilchScriptModule* module = GetActiveModule();
    return ::LoadComposition(module, path, composition);
  }

  bool LoadComposition(JsonLoader& loader, Composition* composition)
  {
    ZilchScriptModule* module = GetActiveModule();
    return ::LoadComposition(module, loader, composition);
  }

  void ProcessFrame()
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
        mTotalFrameTime += dt;
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

  void DrawFrame()
  {
    Zilch::HandleOf<Zilch::EventData> toSend = ZilchAllocate(Zilch::EventData);
    toSend->EventName = Events::EngineUpdate;
    Zilch::EventSend(mEngine, toSend->EventName, toSend);
  }

  void ReloadResources()
  {
    mResourceSystem.ReloadLibraries();
  }

  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
  {
    auto self = (HelloTriangleApplication*)glfwGetWindowUserPointer(window);
    if(key == GLFW_KEY_R && action == GLFW_PRESS)
      self->ReloadResources();
  }

  void mainLoop()
  {
    while(!glfwWindowShouldClose(mWindow))
    {
      glfwPollEvents();
      ProcessFrame();
    }

    GraphicsEngine* graphicsEngine = mEngine->Has<GraphicsEngine>();
    graphicsEngine->WaitIdle();
  }

  void Shutdown() 
  {
    GraphicsEngine* graphicsEngine = mEngine->Has<GraphicsEngine>();
    graphicsEngine->Shutdown();
    glfwDestroyWindow(mWindow);
    glfwTerminate();
  }

  GLFWwindow* mWindow;
  std::chrono::steady_clock::time_point mLastFrameTime;
  double mTotalFrameTime;
};

int main()
{
  Setup setup;
  HelloTriangleApplication app(&setup);

  try
  {
    app.run();
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
