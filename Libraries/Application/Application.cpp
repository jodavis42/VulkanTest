#include "Precompiled.hpp"

#include "Utilities/File.hpp"
#include "Utilities/JsonSerializers.hpp"
#include "Resources/ResourceZilchStaticLibrary.hpp"
#include "Resources/ResourceSystem.hpp"
#include "Resources/ResourceLibrary.hpp"
#include "Engine/LevelManager.hpp"
#include "Graphics/GraphicsZilchStaticLibrary.hpp"
#include "Graphics/GraphicsEngine.hpp"
#include "Graphics/GraphicsSpace.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <chrono>
#include <iostream>

class HelloTriangleApplication {
public:
  void run()
  {
    Initialize();
    mainLoop();
    Shutdown();
  }

private:
  String mResourcesDir;
  String mShaderCoreDir;
  ResourceSystem mResourceSystem;

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

  void InitializeZilch()
  {
    Zilch::ZilchSetup* zilchSetup = new Zilch::ZilchSetup();

    Zilch::Module module;
    Zilch::ExecutableState::CallingState = module.Link();

    ResourceStaticLibrary::InitializeInstance();
    ResourceStaticLibrary::GetInstance().GetLibrary();
    GraphicsStaticLibrary::InitializeInstance();
    GraphicsStaticLibrary::GetInstance().GetLibrary();
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
    mResourceSystem.RegisterResourceManager(ZilchFragmentFile, ZilchFragmentFileManager, new ZilchFragmentFileManager());
    mResourceSystem.RegisterResourceManager(Texture, TextureManager, new TextureManager());
    mResourceSystem.RegisterResourceManager(Mesh, MeshManager, new MeshManager());
    mResourceSystem.RegisterResourceManager(ZilchMaterial, ZilchMaterialManager, new ZilchMaterialManager());
    mResourceSystem.LoadLibrary("BasicProject", Zero::FilePath::Combine(mResourcesDir, "BasicProject"));
  }

  void Initialize()
  {
    InitializeZilch();
    LoadConfiguration();
    InitializeResourceSystem();

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
    mGraphicsEngine.Initialize(graphicsInitData);
    mGraphicsEngine.mWindowSizeQueryFn = [this](size_t& width, size_t& height) {QueryWindowSize(width, height); };

    GraphicsEngineRendererInitData rendererInitData;
    rendererInitData.mInitialWidth = width;
    rendererInitData.mInitialHeight = height;
    rendererInitData.mSurfaceCreationCallback.mCallbackFn = &HelloTriangleApplication::SurfaceCreationCallback;
    rendererInitData.mSurfaceCreationCallback.mUserData = this;
    mGraphicsEngine.InitializeRenderer(rendererInitData);

    mGraphicsEngine.PopulateMaterialBuffer();
    LoadLevel("Level");
  }

  void LoadLevel(const String& levelName)
  {
    LevelManager* levelManager = mResourceSystem.FindResourceManager(LevelManager);
    Level* level = levelManager->FindResource(ResourceName{levelName});
    ReturnIf(level == nullptr, , "Failed to find level '%s'", levelName.c_str());
    
    GraphicsSpace* space = mGraphicsEngine.CreateSpace(levelName);
    String filePath = level->mPath;
    JsonLoader loader;
    loader.LoadFromFile(filePath);

    size_t objCount;
    loader.BeginArray(objCount);
    for(size_t objIndex = 0; objIndex < objCount; ++objIndex)
    {
      loader.BeginArrayItem(objIndex);

      Model* model = new Model();
      LoadModel(loader, model);
      space->Add(model);

      loader.EndArrayItem();
    }
  }

  void DrawFrame(UpdateEvent& toSend)
  {
    for(GraphicsSpace* space : mGraphicsEngine.mSpaces)
    {
      space->Update(toSend);
    }
    mGraphicsEngine.Update();
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

    UpdateEvent toSend;
    toSend.mDt = dt;
    toSend.mTotalTime = mTotalFrameTime;
    DrawFrame(toSend);
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

    mGraphicsEngine.WaitIdle();
  }

  void Shutdown() 
  {
    mGraphicsEngine.Shutdown();
    glfwDestroyWindow(mWindow);
    glfwTerminate();
  }

  GLFWwindow* mWindow;
  GraphicsEngine mGraphicsEngine;
  std::chrono::steady_clock::time_point mLastFrameTime;
  double mTotalFrameTime;
};

int main()
{
  HelloTriangleApplication app;

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
