#pragma once

#include "ResourcesStandard.hpp"

#include "ResourceId.hpp"

struct ResourceExtensionManager
{
  void Register(const ResourceExtension& extension, const ResourceTypeName& typeName)
  {
    mRegisteredExtensions.Insert(extension, typeName);
  }

  ResourceTypeName* Find(const ResourceExtension& extension)
  {
    return mRegisteredExtensions.FindPointer(extension);
  }

  HashMap<ResourceExtension, ResourceTypeName> mRegisteredExtensions;
};

