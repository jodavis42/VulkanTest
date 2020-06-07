#pragma once

#include "EngineStandard.hpp"

#include "Component.hpp"

//-----------------------------------------------------------------------------UpdateEvent
class UpdateEvent : public Zilch::EventData
{
public:
  ZilchDeclareType(UpdateEvent, Zilch::TypeCopyMode::ReferenceType);

  float mDt;
  double mElapsedLogicTime = 0.0;
  double mElapsedFrameTime = 0.0;
};

namespace Events
{
ZilchDeclareEvent(LogicUpdate, UpdateEvent);
ZilchDeclareEvent(FrameUpdate, UpdateEvent);
ZilchDeclareEvent(EngineUpdate, Zilch::EventData);
}//namespace Events

