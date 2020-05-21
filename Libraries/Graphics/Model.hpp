#pragma once

#include "Math.hpp"
#include "GraphicsStandard.hpp"
#include "Graphical.hpp"

class JsonLoader;

struct Model : public Graphical
{
  virtual void FilloutFrameData(GraphicalFrameData& frameData) const override;

  String mMaterialName;
  String mMeshName;

  Vec3 mTranslation = Vec3(0, 0, 0);
  Matrix3 mRotation;
  Vec3 mScale = Vec3(1, 1, 1);
};

void LoadModel(JsonLoader& loader, Model* model);
