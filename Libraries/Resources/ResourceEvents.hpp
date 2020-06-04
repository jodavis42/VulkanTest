#pragma once

#include "ResourcesStandard.hpp"
#include "Zilch/Zilch.hpp"
#include "ResourceMetaFile.hpp"

class Resource;

class ResourceLoadEvent : public Zilch::EventData
{
public:
  ZilchDeclareType(ResourceLoadEvent, Zilch::TypeCopyMode::ReferenceType);
  Resource* mResource = nullptr;
};

namespace Events
{
ZilchDeclareEvent(ResourceLoaded, ResourceLoadEvent);
ZilchDeclareEvent(ResourceReLoaded, ResourceLoadEvent);
}//namespace Events
