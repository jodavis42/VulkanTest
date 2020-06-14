#include "Precompiled.hpp"

#include "ZilchScriptLibrary.hpp"

#include "Resources/ResourceSystem.hpp"
#include "Resources/ResourceLibrary.hpp"
#include "Resources/ResourceExtensions.hpp"
#include "Engine/EngineZilchStaticLibrary.hpp"

#include "ZilchScriptManager.hpp"

//-------------------------------------------------------------------ZilchScriptLibrary
ZilchDefineType(ZilchScriptLibrary, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}

//-------------------------------------------------------------------ZilchScriptManager
ZilchScriptLibraryManager::ZilchScriptLibraryManager(ResourceSystem* resourceSystem)
  : mResourceSystem(resourceSystem)
{
}

ZilchScriptLibraryManager::~ZilchScriptLibraryManager()
{
}

void ZilchScriptLibraryManager::SetNativeDependencies(Zilch::Module* dependencies)
{
  mNativeDependencies = new ZilchModule();
  for(size_t i = 0; i < dependencies->Size(); ++i)
  {
    Zilch::Library* library = (*dependencies)[i];
    ZilchLibrary* zilchLibrary = new ZilchLibrary();
    zilchLibrary->mZilchLibrary = library;
    mNativeDependencies->mDependencies.PushBack(zilchLibrary);
  }
}

void ZilchScriptLibraryManager::BuildLibraries()
{
  ResourceLibraryGraph* libraryGraph = mResourceSystem->GetLibraryGraph();
  for(ResourceLibrary* library : libraryGraph->GetLibraries())
    BuildLibrary(library);

  mModule = new ZilchModule();
  mModule->mDependencies.Insert(mModule->mDependencies.End(), mNativeDependencies->mDependencies.All());
  for(ZilchScriptLibrary* library : mLibraries)
  {
    ZilchLibrary* zilchLibrary = new ZilchLibrary();
    zilchLibrary->Add(library);
    zilchLibrary->mZilchLibrary = library->mZilchLibrary;
    mModule->mDependencies.PushBack(zilchLibrary);
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
  scriptProject.UserData = mNativeDependencies.GetObject();
  
  Zilch::Module dependencies;
  mNativeDependencies->PopulateZilchModule(dependencies);

  Zilch::EventConnect(&scriptProject, Zilch::Events::PreParser, &ResourceLibrary::OnPreParser, resourceLibrary);
  Zilch::EventConnect(&scriptProject, Zilch::Events::TypeParsed, &ZilchScriptLibraryManager::OnTypeParsed, this, this);
  Zilch::EventConnect(&scriptProject, Zilch::Events::CompilationError, &ZilchScriptLibraryManager::OnError, this, this);
  Zilch::LibraryRef library = scriptProject.Compile(resourceLibrary->mLibraryName, dependencies, Zilch::EvaluationMode::Project);
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

ZilchModule* ZilchScriptLibraryManager::GetModule()
{
  return mModule;
}

Array<ZilchScriptLibrary*>::range ZilchScriptLibraryManager::GetLibraries()
{
  return mLibraries.All();
}

void ZilchScriptLibraryManager::OnError(Zilch::ErrorEvent* e)
{
  String msg = e->GetFormattedMessage(Zilch::MessageFormat::MsvcCpp);
  Zilch::Console::WriteLine(msg);
}

void ZilchScriptLibraryManager::OnTypeParsed(Zilch::ParseEvent* e)
{
  Zilch::LibraryBuilder* builder = e->Builder;
  Zilch::BoundType* boundType = e->Type;
  AddComponentExtensions(*builder, boundType);
}

