#pragma once

#include "ZilchScriptStandard.hpp"
#include "Zilch/Zilch.hpp"

class ResourceLibrary;
class ResourceSystem;

//-------------------------------------------------------------------ZilchScriptLibrary
class ZilchScriptLibrary
{
public:
  Zilch::LibraryRef mZilchLibrary;
  ResourceLibrary* mResourceLibrary = nullptr;
};

//-------------------------------------------------------------------ZilchScriptModule
class ZilchScriptModule
{
public:
  Zilch::BoundType* FindType(const String& typeName) const;
  Zilch::BoundType* FindType(Zilch::Library* library, const String& typeName) const;

  Zilch::Module mModule;
};

//-------------------------------------------------------------------ZilchScriptLibraryManager
class ZilchScriptLibraryManager
{
public:
  ZilchScriptLibraryManager(ResourceSystem* resourceSystem);
  ~ZilchScriptLibraryManager();

  void SetNativeDependencies(Zilch::Module* dependencies);
  void BuildLibraries();
  void BuildLibrary(ResourceLibrary* resourceLibrary);

  ZilchScriptModule* GetModule();

private:
  static void OnError(Zilch::ErrorEvent* e, void* userData);
  static void OnTypeParsed(Zilch::ParseEvent* e, void* userData);

  Zilch::Module* mNativeDependencies = nullptr;
  ZilchScriptModule* mModule = nullptr;
  Array<ZilchScriptLibrary*> mLibraries;
  ResourceSystem* mResourceSystem = nullptr;
};
