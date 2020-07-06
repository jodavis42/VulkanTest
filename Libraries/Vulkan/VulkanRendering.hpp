#pragma once
#include "Graphics/GraphicsBufferTypes.hpp"

struct RendererData;
struct FrameBlock;
struct ViewBlock;
struct RenderTaskEvent;
struct RenderGroupRenderTask;
struct RenderQueue;
struct GraphicalFrameData;
class VulkanRenderer;
class VulkanCommandBuffer;

struct GlobalBufferOffset
{
  Array<uint32_t> mFrameNodeOffsets;
  Array<uint32_t> mViewNodeOffsets;
};

void PopulateGlobalBuffers(RendererData& rendererData, const RenderQueue& renderQueue, GlobalBufferOffset& offsets);
void PopulateTransformBuffers(RendererData& rendererData, const ViewBlock& viewBlock, const RenderGroupRenderTask& renderGroupTask);
void PopulateTransformBuffers(VulkanRenderer& renderer, const ViewBlock& viewBlock, const Array<GraphicalFrameData>& frameData);
void AddFrameDataDrawCommands(VulkanRenderer& renderer, VulkanCommandBuffer& commandBuffer, const Array<GraphicalFrameData>& frameData);
void DrawModels(RendererData& rendererData, const ViewBlock& viewBlock, const RenderGroupRenderTask& renderGroupTask);

void ProcessRenderQueue(RendererData& rendererData, const RenderQueue& renderQueue);
