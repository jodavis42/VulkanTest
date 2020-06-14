#pragma once

#include "ApplicationConfig.hpp"

#include "ZilchScript/ZilchScriptLibrary.hpp"
#include "ZilchHelpers/ZilchCallingStateSingleton.hpp"
#include "Resources/ResourceSystem.hpp"
#include "Engine/Composition.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Space.hpp"
#include "Graphics/GraphicsZilchStaticLibrary.hpp"

#include <chrono>

class JsonLoader;
struct GLFWwindow;
class ZilchModule;

class Application
{
public:
  Application(ApplicationConfig* config);

  void Run();
  GLFWwindow* GetWindow();

private:
  void Initialize();
  void Shutdown();
  void LoadConfiguration();
  void InitializeResourceSystem();
  void BuildZilchScripts();
  void BuildEngine();
  void BuildGame();
  void BuildSpace();

  void LoadLevel(const String& levelName);
  bool LoadComposition(const String& path, Composition* composition);
  bool LoadComposition(JsonLoader& loader, Composition* composition);
  void ReloadResources();

  void MainLoop();
  void ProcessFrame();

  ZilchModule* GetActiveModule();
  
  void QueryWindowSize(size_t& outWidth, size_t& outHeight);
  static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void MouseMoveCallback(GLFWwindow* window, double xPos, double yPos);
  static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
  static void MouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
  static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
  
  ApplicationConfig* mConfig = nullptr;
  String mResourcesDir;
  String mShaderCoreDir;
  ResourceSystem mResourceSystem;
  ZilchScriptLibraryManager mZilchScriptLibraryManager;
  ZilchCallingStateSingleton mCallingStateSingleton;

  Zilch::HandleOf<Engine> mEngine;
  Zilch::HandleOf<GameSession> mGame;
  Zilch::HandleOf<Space> mSpace;
  
  GLFWwindow* mWindow;
  std::chrono::steady_clock::time_point mLastFrameTime;
};
