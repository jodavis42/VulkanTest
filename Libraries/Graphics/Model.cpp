#include "Precompiled.hpp"

#include "Model.hpp"

#include "JsonSerializers.hpp"

void LoadModel(JsonLoader& loader, Model* model)
{
  if(loader.BeginMember("Transform"))
  {
    LoadArray(loader, "Translation", model->mTranslation);
    loader.EndMember();
  }
  if(loader.BeginMember("Model"))
  {
    LoadPrimitive(loader, "Mesh", model->mMeshName);
    LoadPrimitive(loader, "Material", model->mMaterialName);
    loader.EndMember();
  }
}