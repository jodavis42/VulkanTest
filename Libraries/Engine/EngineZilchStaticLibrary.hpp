#pragma once

#include "EngineStandard.hpp"
#include "Zilch/Zilch.hpp"

void AddComponentExtensions(Zilch::LibraryBuilder& builder, Zilch::BoundType* boundType);
void AddNativeLibraryExtensions(Zilch::LibraryBuilder& builder);

ZilchDeclareStaticLibrary(EngineStaticLibrary, ZilchNoNamespace, ZeroNoImportExport);
