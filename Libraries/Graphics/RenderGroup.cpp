#include "Precompiled.hpp"

#include "RenderGroup.hpp"

#include "ResourceMetaFile.hpp"

//-------------------------------------------------------------------RenderGroup
ZilchDefineType(RenderGroup, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}

//-------------------------------------------------------------------RenderGroupManager
ZilchDefineType(RenderGroupManager, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();
}

RenderGroupManager::RenderGroupManager()
{
 
}

RenderGroupManager::~RenderGroupManager()
{
}

void RenderGroupManager::GetExtensions(Array<ResourceExtension>& extensions) const
{
  extensions.PushBack({"renderGroup"});
}

bool RenderGroupManager::OnLoadResource(const ResourceMetaFile& resourceMeta, RenderGroup* renderGroup)
{
  return true;
}

bool RenderGroupManager::OnReLoadResource(const ResourceMetaFile& resourceMeta, RenderGroup* renderGroup)
{
  return true;
}

//-------------------------------------------------------------------RenderGroupSet
ZilchDefineType(RenderGroupSet, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}
