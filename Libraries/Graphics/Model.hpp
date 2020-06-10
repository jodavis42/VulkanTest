#pragma once

#include "Math.hpp"
#include "GraphicsStandard.hpp"
#include "Graphical.hpp"

struct Mesh;
struct ZilchMaterial;

//-----------------------------------------------------------------------------Model
struct Model : public Graphical
{
  ZilchDeclareType(Model, Zilch::TypeCopyMode::ReferenceType);

  virtual void Initialize(const CompositionInitializer& initializer) override;
  virtual void OnDestroy() override;
  virtual void FilloutFrameData(GraphicalFrameData& frameData) const override;
  
  Zilch::HandleOf<ZilchMaterial> mMaterial;
  Zilch::HandleOf<Mesh> mMesh;
};
