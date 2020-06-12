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
#include "Engine/GameSession.hpp"
#include "Engine/Keyboard.hpp"
#include "Engine/Mouse.hpp"
#include "Engine/Transform.hpp"
#include "Engine/Space.hpp"
#include "Engine/TimeSpace.hpp"
#include "Graphics/GraphicsEngine.hpp"
#include "Graphics/GraphicsSpace.hpp"
#include "ZilchScript/ZilchScriptManager.hpp"
#include "ZilchScript/ZilchScriptLibrary.hpp"
#include "ZilchScript/ZilchComponent.hpp"
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

void OnConsoleWrite(Zilch::ConsoleEvent* e, void* userData)
{
  printf("%s", e->Text.c_str());
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
  Zilch::EventConnect(&Zilch::Console::Events, Zilch::Events::ConsoleWrite, &OnConsoleWrite, nullptr);
  LoadConfiguration();
  InitializeResourceSystem();
  BuildZilchScripts();
  BuildEngine();
  BuildGame();
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
  glfwSetCursorPosCallback(mWindow, &Application::MouseMoveCallback);
  glfwSetMouseButtonCallback(mWindow, &Application::MouseButtonCallback);
  glfwSetScrollCallback(mWindow, &Application::MouseScrollCallback);

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

void Application::BuildGame()
{
  ArchetypeManager* archetypeManager = mResourceSystem.FindResourceManager(ArchetypeManager);
  Archetype* gameArchetype = archetypeManager->FindResource(ResourceName{"Game"});
  Zilch::HandleOf<GameSession> game = ZilchAllocate(GameSession);

  if(gameArchetype != nullptr)
    LoadComposition(gameArchetype->mPath, game);

  mEngine->Add(game);
  game->Initialize(CompositionInitializer());
  mGame = game;
}

void Application::BuildSpace()
{
  ArchetypeManager* archetypeManager = mResourceSystem.FindResourceManager(ArchetypeManager);
  Archetype* spaceArchetype = archetypeManager->FindResource(ResourceName{"Space"});
  Zilch::HandleOf<Space> space = ZilchAllocate(Space);

  if(spaceArchetype != nullptr)
    LoadComposition(spaceArchetype->mPath, space);

  if(space->Has<TimeSpace>() == nullptr)
    space->AddComponent(ZilchAllocate(TimeSpace));
  if(space->Has<GraphicsSpace>() == nullptr)
    space->AddComponent(ZilchAllocate(GraphicsSpace));

  mGame->Add(space);
  space->Initialize(CompositionInitializer());
  mSpace = space;
}

void Application::LoadLevel(const String& levelName)
{
  ZilchScriptModule* module = GetActiveModule();
  LevelManager* levelManager = mResourceSystem.FindResourceManager(LevelManager);
  ArchetypeManager* archetypeManager = mResourceSystem.FindResourceManager(ArchetypeManager);
  Level* level = levelManager->FindResource(ResourceName{levelName});
  ReturnIf(level == nullptr, , "Failed to find level '%s'", levelName.c_str());

  JsonLoader loader;
  SerializerContext context{module, &mResourceSystem, &loader};
  ::LoadLevel(context, level, mSpace);
  mSpace->InitializeCompositions(CompositionInitializer());
}

bool Application::LoadComposition(const String& path, Composition* composition)
{
  JsonLoader loader;
  SerializerContext context{GetActiveModule(), &mResourceSystem, &loader};
  return ::LoadComposition(context, path, composition);
}

bool Application::LoadComposition(JsonLoader& loader, Composition* composition)
{
  SerializerContext context{GetActiveModule(), &mResourceSystem, &loader};
  return ::LoadComposition(context, composition);
}

void Application::ReloadResources()
{
  mResourceSystem.ReloadLibraries();

  ZilchScriptManager* zilchScriptManager = mResourceSystem.FindResourceManager(ZilchScriptManager);
  // If any zilch scripts were modified then recompile all scripts (could optimize later)
  if(!zilchScriptManager->mModifiedScripts.Empty())
  {
    mZilchScriptLibraryManager.BuildLibraries();
    ZilchScriptModule* zilchScriptModule = mZilchScriptLibraryManager.GetModule();
    for(ZilchScriptLibrary* zilchScriptLibrary : mZilchScriptLibraryManager.GetLibraries())
    {
      if(zilchScriptLibrary->mOldZilchLibrary != nullptr)
        Zilch::ExecutableState::CallingState->PatchLibrary(zilchScriptLibrary->mZilchLibrary, zilchScriptLibrary->mOldZilchLibrary);
    }
    zilchScriptManager->mModifiedScripts.Clear();

    // Have to re-allocate any zilch component otherwise the old library will free the memory when deallocated.
    for(GameSession* game : mEngine->mGameSessions)
    {
      for(Space* space : game->mSpaces)
      {
        for(Composition* composition : space->mCompositions)
        {
          for(size_t i = 0; i < composition->mComponents.Size(); ++i)
          {
            Zilch::HandleOf<Component> component = composition->mComponents[i];
            Zilch::BoundType* boundType = component->ZilchGetDerivedType();
            if(boundType->IsA(ZilchTypeId(ZilchComponent)))
            {
              SerializerContext context{GetActiveModule(), &mResourceSystem, nullptr};
              Zilch::HandleOf<Component> newComponent = nullptr;
              CloneComponent(context, *component.Get<Component*>(), newComponent);
              composition->mComponents[i] = newComponent;
              component.Delete();
              newComponent->mOwner = composition;
              newComponent->Initialize(CompositionInitializer());
            }
          }
        }
      }
    }
  }
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
  
  mEngine->Update(dt);
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
  bool isDown = false;
  if(action == GLFW_PRESS)
    isDown = true;
  else if(action == GLFW_RELEASE)
    isDown = false;
  else
    return;

  Keys::Enum keyValue = Keys::None;
  if(('0' <= key && key <= '9') || ('A' <= key && key <= 'Z'))
    keyValue = (Keys::Enum)key;
  else if(key == GLFW_KEY_UP)
    keyValue = Keys::Up;
  else if(key == GLFW_KEY_DOWN)
    keyValue = Keys::Down;
  else if(key == GLFW_KEY_LEFT)
    keyValue = Keys::Left;
  else if(key == GLFW_KEY_RIGHT)
    keyValue = Keys::Right;
  else if(key == GLFW_KEY_SPACE)
    keyValue = Keys::Space;
  else if(key == GLFW_KEY_TAB)
    keyValue = Keys::Tab;
  else if(key == GLFW_KEY_ENTER)
    keyValue = Keys::Enter;
  else if(key == GLFW_KEY_LEFT_SHIFT)
    keyValue = Keys::Shift;
  else
    keyValue = (Keys::Enum)key;

  auto self = (Application*)glfwGetWindowUserPointer(window);
  Keyboard* keyboard = self->mEngine->Has<Keyboard>();
  if(keyValue != Keys::None)
    keyboard->ProcessKey(keyValue, isDown);
    
  if(key == Keys::R && isDown == true)
  {
    self->ReloadResources();
  }
}

void Application::MouseMoveCallback(GLFWwindow* window, double xPos, double yPos)
{
  auto self = (Application*)glfwGetWindowUserPointer(window);
  Mouse* mouse = self->mEngine->Has<Mouse>();
  if(mouse == nullptr)
    return;

  Vec2 pos;
  pos.x = static_cast<float>(xPos);
  pos.y = static_cast<float>(yPos);
  mouse->ProcessPosition(pos);
}

void Application::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
  auto self = (Application*)glfwGetWindowUserPointer(window);
  Mouse* mouse = self->mEngine->Has<Mouse>();
  if(mouse == nullptr)
    return;

  MouseButtonStates::Enum state = MouseButtonStates::None;
  if(action == GLFW_PRESS)
    state = MouseButtonStates::Down;
  if(action == GLFW_RELEASE)
    state = MouseButtonStates::Up;

  if(button == GLFW_MOUSE_BUTTON_LEFT)
    mouse->ProcessButton(MouseButtons::Left, state);
  else if(button == GLFW_MOUSE_BUTTON_RIGHT)
    mouse->ProcessButton(MouseButtons::Right, state);
  if(button == GLFW_MOUSE_BUTTON_MIDDLE)
    mouse->ProcessButton(MouseButtons::Middle, state);
}

void Application::MouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
  auto self = (Application*)glfwGetWindowUserPointer(window);
  Mouse* mouse = self->mEngine->Has<Mouse>();
  if(mouse == nullptr)
    return;

  Vec2 scroll;
  scroll.x = static_cast<float>(xOffset);
  scroll.y = static_cast<float>(yOffset);
  mouse->ProcessMouseScroll(scroll);
}

void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
  auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
}
