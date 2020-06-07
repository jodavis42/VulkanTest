#include "Precompiled.hpp"

#include "UpdateEvent.hpp"

namespace Events
{
ZilchDefineEvent(LogicUpdate);
ZilchDefineEvent(FrameUpdate);
ZilchDefineEvent(EngineUpdate);
}//namespace Events

//-----------------------------------------------------------------------------UpdateEvent
ZilchDefineType(UpdateEvent, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindField(mDt);
  ZilchBindField(mElapsedLogicTime);
  ZilchBindField(mElapsedFrameTime);
}

