#pragma once

#include "Zilch/Zilch.hpp"
#include "ResourceId.hpp"

class ResourceSystem;
class ResourceLibrary;
class ZilchModule;

void AddResourcePropertyExtension(Zilch::LibraryBuilder& builder, ResourceSystem* resourceSystem, Zilch::BoundType* resourceType, const ResourceName& resourceName);
void AddResourcePropertyExtensions(Zilch::LibraryBuilder& builder, ZilchModule* dependencies, ResourceLibrary* library);

void AddResourceExtensions(Zilch::LibraryBuilder& builder, ResourceSystem* resourceSystem, Zilch::BoundType* resourceType);
void AddResourceExtensions(Zilch::LibraryBuilder& builder);
