#include "Precompiled.hpp"

#include "ApplicationConfig.hpp"

#include "Resources/ResourceZilchStaticLibrary.hpp"
#include "Engine/EngineZilchStaticLibrary.hpp"
#include "Graphics/GraphicsZilchStaticLibrary.hpp"
#include "ZilchScript/ZilchScriptZilchStaticLibrary.hpp"
#include "ZilchScript/ZilchScriptLibrary.hpp"

ApplicationConfig::ApplicationConfig()
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

ApplicationConfig::~ApplicationConfig()
{

}
