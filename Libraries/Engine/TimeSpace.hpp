#pragma once

#include "EngineStandard.hpp"

#include "Component.hpp"

//-------------------------------------------------------------------TimeSpace
struct TimeSpace : public Component
{
public:
  ZilchDeclareType(TimeSpace, Zilch::TypeCopyMode::ReferenceType);

  void Update(float dt);
  void SendEvent(const String& eventName);
  float GetFrameRate() const;

  bool mPaused = false;
  float mFramesPerSecond = 60.0f;
  float mTimeAcculated = 0;
  double mElapsedLogicTime = 0.0;
  double mElapsedFrameTime = 0.0;
};
