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

//-----------------------------------------------------------------------------Model
ZilchDefineType(Model, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindField(mMaterialName)->AddAttribute("Serialize")->AddParameter(String("Material"));
  ZilchBindField(mMeshName)->AddAttribute("Serialize")->AddParameter(String("Mesh"));
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
  frameData.mMesh = engine->mMeshManager->FindResource(ResourceName{mMeshName});
  frameData.mZilchMaterial = engine->mZilchMaterialManager->FindResource(ResourceName{mMaterialName});
  frameData.mZilchShader = engine->mZilchShaderManager.Find(mMaterialName);

  Transform* transform = GetOwner()->Has<Transform>();
  frameData.mLocalToWorld = transform->GetWorldMatrix();
}
