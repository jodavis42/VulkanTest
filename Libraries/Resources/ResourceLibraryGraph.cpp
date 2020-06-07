#include "Precompiled.hpp"

#include "ResourceLibraryGraph.hpp"

#include "ResourceLibrary.hpp"

ResourceLibraryGraph::ResourceLibraryGraph()
{

}

ResourceLibraryGraph::~ResourceLibraryGraph()
{
  for(ResourceLibrary* library : GetLibraries())
    DestroyLibrary(library);
  mLibraryStack.Clear();
}

void ResourceLibraryGraph::PushLibrary(ResourceLibrary* library)
{
  mLibraryStack.PushBack(library);
}

void ResourceLibraryGraph::PopLibrary()
{
  if(mLibraryStack.Empty())
    return;

  ResourceLibrary* library = mLibraryStack.Back();
  mLibraryStack.PopBack();
  DestroyLibrary(library);
}

ResourceLibraryGraph::Range ResourceLibraryGraph::GetLibraries()
{
  return Range(mLibraryStack.All());
}

void ResourceLibraryGraph::DestroyLibrary(ResourceLibrary* library)
{
  delete library;
}
