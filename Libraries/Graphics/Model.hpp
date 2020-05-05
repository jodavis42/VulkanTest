#pragma once

#include "Math.hpp"
#include "GraphicsStandard.hpp"

class JsonLoader;

struct Model
{
  String mMaterialName;
  String mMeshName;

  Vec3 mTranslation = Vec3(0, 0, 0);
  Matrix3 mRotation;
  Vec3 mScale = Vec3(1, 1, 1);
};

void LoadModel(JsonLoader& loader, Model* model);
