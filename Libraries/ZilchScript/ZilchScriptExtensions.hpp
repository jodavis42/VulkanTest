#pragma once

#include "ZilchScriptStandard.hpp"
#include "ZilchScriptExtensions.hpp"

#include "Zilch/Zilch.hpp"

class Composition;

//-------------------------------------------------------------------ZilchScriptExtensions
struct ZilchScriptExtensions
{
public:
  ZilchDeclareType(ZilchScriptExtensions, Zilch::TypeCopyMode::ReferenceType);

  static void Connect(Composition* sender, const String& eventName, const Zilch::Delegate& delegate);
  static void Disconnect(Composition* sender, const String& eventName, const Zilch::Delegate& delegate);
};
