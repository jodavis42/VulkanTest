#pragma once
#include "Graphics/GraphicsBufferTypes.hpp"

struct VulkanRuntimeData;
struct ModelRenderData;
struct RendererData;

struct VulkanGlobalBufferData
{
  const FrameData* mFrameData = nullptr;
  const CameraData* mCameraData = nullptr;
};
struct VulkanTransformBufferData
{
  Matrix4 mWorldToView;
  Matrix4 mViewToPerspective;
  Array<ModelRenderData>* mModelRenderData = nullptr;
};

void PopulateGlobalBuffers(RendererData& rendererData, VulkanGlobalBufferData& globalBufferData);
void PopulateTransformBuffers(RendererData& rendererData, VulkanTransformBufferData& modelRenderData);
void DrawModels(RendererData& rendererData, VulkanTransformBufferData& modelRenderData);
