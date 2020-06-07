#pragma once

#include "EngineStandard.hpp"
#include "Zilch/Zilch.hpp"

class JsonLoader;
class Composition;
class Space;
struct Level;
class ZilchScriptModule;

bool LoadProperty(JsonLoader& loader, Zilch::Type* propertyType, const String& propertyName, Zilch::Any& result);
bool LoadProperty(JsonLoader& loader, Zilch::Property* zilchProperty, Zilch::Handle objectInstanceHandle);
bool LoadComposition(ZilchScriptModule* module, const String& path, Composition* composition);
bool LoadComposition(ZilchScriptModule* module, JsonLoader& loader, Composition* composition);
bool LoadLevel(ZilchScriptModule* module, Level* level, Space* space);
