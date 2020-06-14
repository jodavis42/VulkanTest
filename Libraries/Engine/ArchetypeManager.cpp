#include "Precompiled.hpp"

#include "ArchetypeManager.hpp"

#include "ResourceMetaFile.hpp"

//-------------------------------------------------------------------Archetype
ZilchDefineType(Archetype, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}

//-------------------------------------------------------------------ArchetypeManager
ZilchDefineType(ArchetypeManager, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();
}

ArchetypeManager::ArchetypeManager()
{
 
}

ArchetypeManager::~ArchetypeManager()
{
}

void ArchetypeManager::GetExtensions(Array<ResourceExtension>& extensions) const
{
  extensions.PushBack({"archetype"});
}

bool ArchetypeManager::OnLoadResource(const ResourceMetaFile& resourceMeta, Archetype* archetype)
{
  return true;
}

bool ArchetypeManager::OnReLoadResource(const ResourceMetaFile& resourceMeta, Archetype* archetype)
{
  return false;
}
