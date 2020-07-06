#pragma once

#include "GraphicsStandard.hpp"

#include "Engine/Component.hpp"
#include "Graphics/RenderTasks.hpp"

struct RenderTaskEvent;
class GraphicsSpace;
class VulkanRenderer;
class VulkanCommandBuffer;
struct ViewBlock;

//-------------------------------------------------------------------SimpleRendererComponent
struct SimpleRendererComponent : public Component
{
  ZilchDeclareType(SimpleRendererComponent, Zilch::TypeCopyMode::ReferenceType);

  virtual void Initialize(const CompositionInitializer& initializer) override;
  void OnCollectRenderTasks(RenderTaskEvent* renderTaskEvent);

  void CollectFrameData(GraphicsSpace* graphicsSpace, Array<GraphicalFrameData>& frameData);
  void UploadBuffers(VulkanRenderer& renderer, ViewBlock& viewBlock, Array<GraphicalFrameData>& frameData);
  void AddGraphicalDrawCommands(VulkanRenderer& renderer, VulkanCommandBuffer& commandBuffer, Array<GraphicalFrameData>& frameData);

  bool mActive = true;
};
