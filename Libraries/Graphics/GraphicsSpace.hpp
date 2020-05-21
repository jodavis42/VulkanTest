#pragma once

#include "GraphicsStandard.hpp"

struct Camera;
struct Model;
struct GraphicsEngine;
struct RenderFrame;
struct RenderQueue;

struct UpdateEvent
{
  float mDt = 0.0f;
  double mTotalTime = 0.0;
};

class GraphicsSpace
{
public:
  GraphicsSpace();
  ~GraphicsSpace();
  void Add(Model* model);
  void Update(UpdateEvent& e);
  void RenderQueueUpdate(RenderQueue& renderQueue);

  float mTotalTimeElapsed = 0.0;
  Array<Camera*> mCameras;
  Array<Model*> mModels;
  String mName;
  GraphicsEngine* mEngine = nullptr;
};
