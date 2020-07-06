#pragma once

#include "Math.hpp"
#include "Graphics/GraphicsBufferTypes.hpp"

struct Mesh;
struct Texture;
struct ZilchShader;
struct ZilchMaterial;
struct ZilchShaderManager;
struct ZilchMaterialManager;
struct RenderQueue;

struct MaterialBatchUploadData
{
  struct MaterialData
  {
    const ZilchShader* mZilchShader = nullptr;
    const ZilchMaterial* mZilchMaterial = nullptr;
  };
  const ZilchShaderManager* mZilchShaderManager = nullptr;
  const ZilchMaterialManager* mZilchMaterialManager = nullptr;
  Array<MaterialData> mMaterials;
};

struct Renderer
{
public:
  virtual ~Renderer();

  virtual void Shutdown() abstract;
  virtual void Destroy() abstract;

  virtual void CreateMesh(const Mesh* mesh) abstract;
  virtual void DestroyMesh(const Mesh* mesh) abstract;

  virtual void CreateTexture(const Texture* texture) abstract;
  virtual void DestroyTexture(const Texture* texture) abstract;

  virtual void CreateShader(const ZilchShader* zilchShader) abstract;
  virtual void DestroyShader(const ZilchShader* zilchShader) abstract;

  virtual void CreateShaderMaterial(ZilchShader* shaderMaterial) abstract;
  virtual void UpdateShaderMaterialInstance(const ZilchShader* zilchShader, const ZilchMaterial* zilchMaterial) abstract;
  virtual void UploadShaderMaterialInstances(MaterialBatchUploadData& materialBatchUploadData) abstract;
  virtual void DestroyShaderMaterial(const ZilchShader* zilchShader) abstract;

  virtual void DrawRenderQueue(RenderQueue& renderQueue) abstract;
  virtual void WaitForIdle() abstract;

  virtual void BeginReshape() abstract;
  virtual void Reshape(size_t width, size_t height, float aspectRatio) abstract;
  virtual void EndReshape() abstract;
  virtual void GetShape(size_t& width, size_t& height, float& aspectRatio) const abstract;

  virtual Matrix4 BuildPerspectiveMatrix(float verticalFov, float aspectRatio, float nearDistance, float farDistance) const abstract;
};
