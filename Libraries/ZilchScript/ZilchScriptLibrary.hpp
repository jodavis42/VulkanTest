#pragma once

#include "ZilchScriptStandard.hpp"
#include "ZilchHelpers/ZilchModule.hpp"
#include "Zilch/Zilch.hpp"

class ResourceLibrary;
class ResourceSystem;

//-------------------------------------------------------------------ZilchScriptLibrary
class ZilchScriptLibrary
{
public:
  ZilchDeclareType(ZilchScriptLibrary, Zilch::TypeCopyMode::ReferenceType);

  Zilch::LibraryRef mOldZilchLibrary;
  Zilch::LibraryRef mZilchLibrary;
  ResourceLibrary* mResourceLibrary = nullptr;
};

//-------------------------------------------------------------------ZilchScriptLibraryManager
class ZilchScriptLibraryManager : public Zilch::EventHandler
{
public:
  ZilchScriptLibraryManager(ResourceSystem* resourceSystem);
  ~ZilchScriptLibraryManager();

  void SetNativeDependencies(Zilch::Module* dependencies);
  void BuildLibraries();
  void BuildLibrary(ResourceLibrary* resourceLibrary);
  ZilchScriptLibrary* FindLibrary(ResourceLibrary* resourceLibrary);

  ZilchModule* GetModule();
  Array<ZilchScriptLibrary*>::range GetLibraries();

private:
  void OnError(Zilch::ErrorEvent* e);
  void OnTypeParsed(Zilch::ParseEvent* e);

  Zilch::Ref<ZilchModule> mNativeDependencies;
  Zilch::Ref<ZilchModule> mModule;
  Array<ZilchScriptLibrary*> mLibraries;
  ResourceSystem* mResourceSystem = nullptr;
};
