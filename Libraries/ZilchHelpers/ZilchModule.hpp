#pragma once

#include "ZilchHelpersStandard.hpp"
#include "Zilch/Zilch.hpp"

class ZilchLibrary;
class ZilchModule;

//-------------------------------------------------------------------ZilchModule
class ZilchModule : public Zilch::ReflectionObject
{
public:
  ZilchRefLink(ZilchModule);

  virtual Zilch::Library* GetOwningLibrary() override;
  Zilch::BoundType* FindType(const String& typeName) const;
  void PopulateZilchModule(Zilch::Module& zilchDependencies);

  using ZilchLibraryRef = Zilch::Ref<ZilchLibrary>;
  Array<ZilchLibraryRef> mDependencies;
};

//-------------------------------------------------------------------ZilchLibrary
class ZilchLibrary : public Zilch::ReflectionObject
{
public:
  ZilchRefLink(ZilchLibrary);

  virtual Zilch::Library* GetOwningLibrary() override;
  Zilch::BoundType* FindType(const String& typeName) const;

  Zilch::LibraryRef mZilchLibrary;
  Zilch::Ref<ZilchModule> mModule;
};
