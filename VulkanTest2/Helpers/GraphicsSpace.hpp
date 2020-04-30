#pragma once

#include "Helpers/Math.hpp"

#include <vector>
#include <string>
using String = std::string;

struct Model;
struct GraphicsEngine;
struct RenderFrame;

struct PerCameraData
{
  alignas(16) Matrix4 view;
  alignas(16) Matrix4 proj;
};

struct PerObjectData
{
  alignas(16) Matrix4 model;
};

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
  std::vector<Model*> mModels;
  String mName;
  GraphicsEngine* mEngine = nullptr;
};
