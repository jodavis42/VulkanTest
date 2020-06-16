#pragma once

#include "GraphicsStandard.hpp"

#include "ResourceManager.hpp"
#include "ResourceSet.hpp"

class ResourceMetaFile;

//-------------------------------------------------------------------RenderGroup
class RenderGroup : public Resource
{
public:
  ZilchDeclareType(RenderGroup, Zilch::TypeCopyMode::ReferenceType);
};

//-------------------------------------------------------------------RenderGroupManager
class RenderGroupManager : public ResourceManagerTyped<RenderGroup>
{
public:
  ZilchDeclareType(RenderGroupManager, Zilch::TypeCopyMode::ReferenceType);

  RenderGroupManager();
  ~RenderGroupManager();

  virtual void GetExtensions(Array<ResourceExtension>& extensions) const override;
  virtual bool OnLoadResource(const ResourceMetaFile& resourceMeta, RenderGroup* renderGroup) override;
  virtual bool OnReLoadResource(const ResourceMetaFile& resourceMeta, RenderGroup* renderGroup) override;
};

//-------------------------------------------------------------------RenderGroupSet
class RenderGroupSet : public TypedResourceSet<RenderGroup>
{
public:
  ZilchDeclareType(RenderGroupSet, Zilch::TypeCopyMode::ReferenceType);
};
