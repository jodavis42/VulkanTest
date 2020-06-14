#pragma once

#include "Resources/ResourcesStandard.hpp"
#include "Zilch/Zilch.hpp"

class JsonLoader;
class Composition;
class Component;
class Space;
struct Level;
class ZilchModule;
class ResourceSystem;

struct SerializerContext
{
  ZilchModule* mModule = nullptr;
  ResourceSystem* mResourceSystem = nullptr;
  JsonLoader* mLoader = nullptr;
};

bool LoadProperty(SerializerContext& context, Zilch::Type* propertyType, const String& propertyName, Zilch::Any& result);
bool LoadProperty(SerializerContext& context, Zilch::Property* zilchProperty, Zilch::Handle objectInstanceHandle);
bool LoadComponent(SerializerContext& context, const String& componentName, Composition* compositionOwner);
bool CloneComponent(SerializerContext& context, Component& oldComponent, Zilch::HandleOf<Component>& newComponent);
bool LoadComposition(SerializerContext& context, const String& path, Composition* composition);
bool LoadComposition(SerializerContext& context, Composition* composition);
bool LoadLevel(SerializerContext& context, Level* level, Space* space);

class EngineSerializationSystem
{
  SerializerContext mContext;
};
