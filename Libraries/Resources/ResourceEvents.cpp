#include "Precompiled.hpp"

#include "ResourceEvents.hpp"

namespace Events
{
ZilchDefineEvent(ResourceLoaded);
ZilchDefineEvent(ResourceReLoaded);
}//namespace Events

ZilchDefineType(ResourceLoadEvent, builder, type)
{
}
