#include "Precompiled.hpp"

#include "TimeSpace.hpp"

#include "Composition.hpp"
#include "UpdateEvent.hpp"

//-----------------------------------------------------------------------------TimeSpace
ZilchDefineType(TimeSpace, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindField(mFramesPerSecond);
  ZilchBindField(mPaused);
}

void TimeSpace::Update(float dt)
{
  float framerate = GetFrameRate();

  mElapsedFrameTime += dt;
  if(!mPaused)
  {
    mTimeAcculated += dt;
    if(mTimeAcculated >= framerate)
    {
      mTimeAcculated -= framerate;
      mElapsedLogicTime += framerate;
      SendEvent(Events::LogicUpdate);
    }
  }
  SendEvent(Events::FrameUpdate);
}

void TimeSpace::SendEvent(const String& eventName)
{
  Zilch::HandleOf<UpdateEvent> toSend = ZilchAllocate(UpdateEvent);
  toSend->EventName = eventName;
  toSend->mDt = GetFrameRate();
  toSend->mElapsedLogicTime = mElapsedLogicTime;
  toSend->mElapsedFrameTime = mElapsedFrameTime;
  Zilch::EventSend(GetOwner(), toSend->EventName, toSend);
}

float TimeSpace::GetFrameRate() const
{
  return 1.0f / mFramesPerSecond;
}
