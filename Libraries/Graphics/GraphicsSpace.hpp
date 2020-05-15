#pragma once

#include "GraphicsStandard.hpp"

struct Model;
struct GraphicsEngine;
struct RenderFrame;

struct UpdateEvent
{
  float mDt = 0.0f;
  double mTotalTime = 0.0;
};

class GraphicsSpace
{
public:
  void Update(UpdateEvent& e);
  void UpdateGlobalBuffer(uint32_t frameId);
  void Draw(UpdateEvent& toSend);
  void PrepareFrame(RenderFrame& renderFrame);

  float mTotalTimeElapsed = 0.0;
  Array<Model*> mModels;
  String mName;
  GraphicsEngine* mEngine = nullptr;
};
