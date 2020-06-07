#include "Precompiled.hpp"

#include "ZilchScriptExtensions.hpp"

#include "Engine/Composition.hpp"

//-------------------------------------------------------------------ZilchScriptExtensions
ZilchDefineType(ZilchScriptExtensions, builder, type)
{
  ZilchBindOverloadedMethod(Connect, ZilchStaticOverload(void, Composition*, const String&, const Zilch::Delegate&));
  ZilchBindOverloadedMethod(Disconnect, ZilchStaticOverload(void, Composition*, const String&, const Zilch::Delegate&));
}

void ZilchScriptExtensions::Connect(Composition* sender, const String& eventName, const Zilch::Delegate& delegate)
{
  Zilch::EventsClass::Connect(sender, eventName, delegate);
}

void ZilchScriptExtensions::Disconnect(Composition* sender, const String& eventName, const Zilch::Delegate& delegate)
{
  Zilch::EventHandler* eventHandler = delegate.ThisHandle.Get<Zilch::EventHandler*>();
  if(eventHandler == nullptr)
    return;
  Zilch::EventDisconnect(sender, eventHandler, eventName, eventHandler);
}
