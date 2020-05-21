#include "Precompiled.hpp"

#include "Model.hpp"
#include "GraphicsSpace.hpp"
#include "GraphicsEngine.hpp"
#include "JsonSerializers.hpp"
#include "RenderTasks.hpp"
#include "Mesh.hpp"
#include "ZilchMaterial.hpp"
#include "ZilchShader.hpp"

void Model::FilloutFrameData(GraphicalFrameData& frameData) const
{
  GraphicsEngine* engine = mSpace->mEngine;
  frameData.mMesh = engine->mMeshManager.Find(mMeshName);
  frameData.mZilchMaterial = engine->mZilchMaterialManager.Find(mMaterialName);
  frameData.mZilchShader = engine->mZilchShaderManager.Find(mMaterialName);

  Matrix3 rotation = Matrix3::GenerateRotation(Vec3(0, 0, 1), 0 * Math::DegToRad(90.0f));
  frameData.mLocalToWorld = Matrix4::GenerateTransform(mTranslation, rotation, mScale);
}

void LoadModel(JsonLoader& loader, Model* model)
{
  if(loader.BeginMember("Transform"))
  {
    LoadArray<Vec3, 3>(loader, "Translation", model->mTranslation);
    loader.EndMember();
  }
  if(loader.BeginMember("Model"))
  {
    LoadPrimitive(loader, "Mesh", model->mMeshName);
    LoadPrimitive(loader, "Material", model->mMaterialName);
    loader.EndMember();
  }
}