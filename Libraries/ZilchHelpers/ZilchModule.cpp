#include "Precompiled.hpp"

#include "ZilchModule.hpp"

//-------------------------------------------------------------------ZilchModule
Zilch::Library* ZilchModule::GetOwningLibrary()
{
  return nullptr;
}

Zilch::BoundType* ZilchModule::FindType(const String& typeName) const
{
  Zilch::BoundType* result = nullptr;
  for(auto range = mDependencies.All(); !range.Empty(); range.PopFront())
  {
    ZilchLibrary* library = range.Front();
    result = library->FindType(typeName);
    if(result != nullptr)
      return result;
  }
  return result;
}

void ZilchModule::PopulateZilchModule(Zilch::Module& zilchDependencies)
{
  Zero::HashSet<Zilch::Library*> visitedDependencies;
  for(Zilch::Library* library : zilchDependencies.All())
    visitedDependencies.Insert(library);

  // Handle having no dependencies
  if(mDependencies.Empty())
    return;

  // The shader dependencies contains a whole bunch of top-level dependencies.
  // First add all of these to a stack of modules we need to walk.
  Array<ZilchLibrary*> dependencyStack;
  for(ZilchLibrary* zilchLibrary : mDependencies)
    dependencyStack.PushBack(zilchLibrary);

  // Now we need to iterate over all dependencies of dependencies but in a breadth first order. This is not a "proper"
  // dependency walker and can run into errors in diamond situations, but I'm ignoring this for now.
  for(size_t i = 0; i < dependencyStack.Size(); ++i)
  {
    // Add the zilch dependency of this library to the zilch module
    ZilchLibrary* dependencyLibrary = dependencyStack[i];
    // If we've already walked this library then ignore it
    if(visitedDependencies.Contains(dependencyLibrary->mZilchLibrary))
      continue;

    visitedDependencies.Insert(dependencyLibrary->mZilchLibrary);
    zilchDependencies.PushBack(dependencyLibrary->mZilchLibrary);

    // If this shader library doesn't have any dependencies then stop
    if(dependencyLibrary->mModule == nullptr)
      continue;

    // Otherwise walk all dependent shader libraries
    for(ZilchLibrary* subDependency : dependencyLibrary->mModule->mDependencies)
    {
      dependencyStack.PushBack(subDependency);
    }
  }
}

//-------------------------------------------------------------------ZilchLibrary
Zilch::Library* ZilchLibrary::GetOwningLibrary()
{
  return nullptr;

}
Zilch::BoundType* ZilchLibrary::FindType(const String& typeName) const
{
  Zilch::BoundType* result = mZilchLibrary->BoundTypes.FindValue(typeName, nullptr);
  if(result == nullptr && mModule != nullptr)
  {
    result = mModule->FindType(typeName);
  }
  return result;
}
