#include "Precompiled.hpp"

#include "ZilchScriptLibrary.hpp"

#include "Resources/ResourceSystem.hpp"
#include "Resources/ResourceLibrary.hpp"
#include "Engine/EngineZilchStaticLibrary.hpp"

#include "ZilchScriptManager.hpp"

//-------------------------------------------------------------------ZilchScriptModule
Zilch::BoundType* ZilchScriptModule::FindType(const String& typeName) const
{
  Zilch::BoundType* result = nullptr;
  for(auto range = mModule.All(); !range.Empty(); range.PopFront())
  {
    Zilch::Library* library = range.Front();
    result = FindType(library, typeName);
    if(result != nullptr)
      return result;
  }
  return result;
}

Zilch::BoundType* ZilchScriptModule::FindType(Zilch::Library* library, const String& typeName) const
{
  return library->BoundTypes.FindValue(typeName, nullptr);
}

//-------------------------------------------------------------------ZilchScriptManager
ZilchScriptLibraryManager::ZilchScriptLibraryManager(ResourceSystem* resourceSystem)
  : mResourceSystem(resourceSystem)
{
  mModule = new ZilchScriptModule();
}

ZilchScriptLibraryManager::~ZilchScriptLibraryManager()
{
  delete mModule;
}

void ZilchScriptLibraryManager::SetNativeDependencies(Zilch::Module* dependencies)
{
  mNativeDependencies = dependencies;
}

void ZilchScriptLibraryManager::BuildLibraries()
{
  ResourceLibraryGraph* libraryGraph = mResourceSystem->GetLibraryGraph();
  for(ResourceLibrary* library : libraryGraph->GetLibraries())
    BuildLibrary(library);

  mModule->mModule.Clear();
  mModule->mModule.Append(mNativeDependencies->All());
  for(ZilchScriptLibrary* library : mLibraries)
  {
    mModule->mModule.PushBack(library->mZilchLibrary);
  }
}

void ZilchScriptLibraryManager::BuildLibrary(ResourceLibrary* resourceLibrary)
{
  ZilchScriptManager* zilchScriptManager = mResourceSystem->FindResourceManager(ZilchScriptManager);
  
  Zilch::Project scriptProject;
  for(ResourceId resourceId : resourceLibrary->AllResourcesOfType(ResourceTypeName{"ZilchScript"}))
  {
    ZilchScript* zilchScript = zilchScriptManager->FindResource(resourceId);
    if(zilchScript != nullptr)
      scriptProject.AddCodeFromString(zilchScript->mScriptContents, zilchScript->mPath, zilchScript);
  }
  
  Zilch::EventConnect(&scriptProject, Zilch::Events::TypeParsed, OnTypeParsed, this, nullptr);
  Zilch::EventConnect(&scriptProject, Zilch::Events::CompilationError, OnError, this, nullptr);
  Zilch::LibraryRef library = scriptProject.Compile(resourceLibrary->mLibraryName, *mNativeDependencies, Zilch::EvaluationMode::Project);
  if(library == nullptr)
    return;
  
  ZilchScriptLibrary* zilchScriptLibrary = FindLibrary(resourceLibrary);
  if(zilchScriptLibrary == nullptr)
  {
    zilchScriptLibrary = new ZilchScriptLibrary();
    mLibraries.PushBack(zilchScriptLibrary);
  }
  zilchScriptLibrary->mResourceLibrary = resourceLibrary;
  zilchScriptLibrary->mOldZilchLibrary = zilchScriptLibrary->mZilchLibrary;
  zilchScriptLibrary->mZilchLibrary = library;
  
}

ZilchScriptLibrary* ZilchScriptLibraryManager::FindLibrary(ResourceLibrary* resourceLibrary)
{
  for(size_t i = 0; i < mLibraries.Size(); ++i)
  {
    if(mLibraries[i]->mResourceLibrary == resourceLibrary)
      return mLibraries[i];
  }
  return nullptr;
}

ZilchScriptModule* ZilchScriptLibraryManager::GetModule()
{
  return mModule;
}

Array<ZilchScriptLibrary*>::range ZilchScriptLibraryManager::GetLibraries()
{
  return mLibraries.All();
}

void ZilchScriptLibraryManager::OnError(Zilch::ErrorEvent* e, void* userData)
{
  String msg = e->GetFormattedMessage(Zilch::MessageFormat::MsvcCpp);
  Zilch::Console::WriteLine(msg);
}

void ZilchScriptLibraryManager::OnTypeParsed(Zilch::ParseEvent* e, void* userData)
{
  Zilch::LibraryBuilder* builder = e->Builder;
  Zilch::BoundType* boundType = e->Type;
  AddComponentExtensions(*builder, boundType);
}

