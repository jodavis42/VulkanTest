#include "Precompiled.hpp"

#include "Model.hpp"
#include "GraphicsSpace.hpp"
#include "GraphicsEngine.hpp"
#include "RenderTasks.hpp"
#include "Mesh.hpp"
#include "ZilchMaterial.hpp"
#include "ZilchShader.hpp"
#include "Space.hpp"
#include "Transform.hpp"
#include "Mesh.hpp"

//-----------------------------------------------------------------------------Model
ZilchDefineType(Model, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindFieldProperty(mMaterial);
  ZilchBindFieldProperty(mMesh);
}

void Model::Initialize(const CompositionInitializer& initializer)
{
  Space* space = GetSpace();
  GraphicsSpace* graphicsSpace = space->Has<GraphicsSpace>();
  graphicsSpace->Add(this);
}

void Model::OnDestroy()
{
  GraphicsSpace* graphicsSpace = GetSpace()->Has<GraphicsSpace>();
  graphicsSpace->Remove(this);
}

void Model::FilloutFrameData(GraphicalFrameData& frameData) const
{
  GraphicsEngine* engine = mSpace->mEngine;
  frameData.mMesh = mMesh;
  frameData.mZilchMaterial = mMaterial;
  if(frameData.mZilchMaterial != nullptr)
    frameData.mZilchShader = engine->mZilchShaderManager.Find(frameData.mZilchMaterial->mMaterialName);

  Transform* transform = GetOwner()->Has<Transform>();
  frameData.mLocalToWorld = transform->GetWorldMatrix();
}
