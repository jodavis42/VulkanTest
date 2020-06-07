#pragma once

#include "Math.hpp"
#include "GraphicsStandard.hpp"
#include "Graphical.hpp"

//-----------------------------------------------------------------------------Model
struct Model : public Graphical
{
  ZilchDeclareType(Model, Zilch::TypeCopyMode::ReferenceType);

  virtual void Initialize(const CompositionInitializer& initializer) override;
  virtual void OnDestroy() override;
  virtual void FilloutFrameData(GraphicalFrameData& frameData) const override;
  
  String mMaterialName;
  String mMeshName;
};
