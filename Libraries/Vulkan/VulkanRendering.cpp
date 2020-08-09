#include "Precompiled.hpp"

#include "VulkanRendering.hpp"

#include "VulkanRenderer.hpp"
#include "VulkanInitialization.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanFrameBuffer.hpp"
#include "VulkanImageView.hpp"
#include "VulkanMaterials.hpp"
#include "RenderGraph.hpp"
#include "RenderQueue.hpp"
#include "VulkanShaders.hpp"

uint32_t GetFrameId(RendererData& rendererData)
{
  return rendererData.mRuntimeData->mCurrentImageIndex;
}

uint32_t GetFrameId(VulkanRenderer& renderer)
{
  VulkanRuntimeData& runtimeData = *renderer.mInternal;
  uint32_t frameId = runtimeData.mCurrentImageIndex;
  return frameId;
}

void PopulateGlobalBuffers(RendererData& rendererData, const RenderQueue& renderQueue, GlobalBufferOffset& offsets)
{
  VulkanRenderer& renderer = *rendererData.mRenderer;
  uint32_t frameId = GetFrameId(rendererData);

  offsets.mFrameNodeOffsets.Reserve(renderQueue.mFrameBlocks.Size());
  offsets.mViewNodeOffsets.Reserve(renderQueue.mViewBlocks.Size());

  size_t offset = 0;
  byte* data = static_cast<byte*>(renderer.MapPerFrameUniformBufferMemory(GlobalsBufferName, 0, frameId));
  for(const FrameBlock& frameBlock : renderQueue.mFrameBlocks)
  {
    offsets.mFrameNodeOffsets.PushBack(static_cast<uint32_t>(offset));
    FrameData frameData;
    frameData.mFrameTime = frameBlock.mFrameTime;
    frameData.mLogicTime = frameBlock.mLogicTime;
    memcpy(data, &frameData, sizeof(frameData));
    offset += renderer.AlignUniformBufferOffset(sizeof(frameData));
  }
  for(const ViewBlock& viewBlock : renderQueue.mViewBlocks)
  {
    offsets.mViewNodeOffsets.PushBack(static_cast<uint32_t>(offset));
    CameraData cameraData;
    cameraData.mNearPlane = viewBlock.mNearPlane;
    cameraData.mFarPlane = viewBlock.mFarPlane;
    cameraData.mViewportSize = viewBlock.mViewportSize;
    offset += renderer.AlignUniformBufferOffset(sizeof(cameraData));
  }
  renderer.UnMapPerFrameUniformBufferMemory(GlobalsBufferName, 0, frameId);
}

void PopulateTransformBuffers(VulkanRenderer& renderer, const ViewBlock& viewBlock, const Array<GraphicalFrameData>& frameData)
{
  uint32_t frameId = GetFrameId(renderer);
  TransformData transformData;

  transformData.mPerspectiveToApiPerspective.SetIdentity();
  transformData.mWorldToView = viewBlock.mWorldToView;
  transformData.mViewToPerspective = viewBlock.mViewToPerspective;

  byte* data = static_cast<byte*>(renderer.MapPerFrameUniformBufferMemory(TransformsBufferName, 0, frameId));

  size_t offset = 0;
  size_t objCount = frameData.Size();
  for(size_t i = 0; i < objCount; ++i)
  {
    const GraphicalFrameData& graphicalFrameData = frameData[i];
    transformData.mLocalToWorld = graphicalFrameData.mLocalToWorld;
    byte* memory = data + offset + renderer.AlignUniformBufferOffset(sizeof(TransformData)) * i;
    memcpy(memory, &transformData, sizeof(transformData));
  }
  renderer.UnMapPerFrameUniformBufferMemory(TransformsBufferName, 0, frameId);
}

void AddFrameDataDrawCommands(VulkanRenderer& renderer, VulkanCommandBuffer& commandBuffer, const RenderRanges& renderRanges, const Array<GraphicalFrameData>& frameData)
{
  VkCommandBuffer vkCommandBuffer = commandBuffer.GetVulkanCommandBuffer();

  uint32_t frameId = GetFrameId(renderer);
  uint32_t dynamicOffsets[1] =
  {
    static_cast<uint32_t>(renderer.AlignUniformBufferOffset(sizeof(TransformData)))
  };
  uint32_t baseOffset = 0;
  uint32_t dynamicOffsetsBase[1] = {baseOffset};
  uint32_t dynamicOffsetsCount = 1;
  for(size_t rangeIndex = 0; rangeIndex < renderRanges.mRenderRanges.Size(); ++rangeIndex)
  {
    const RenderRanges::RenderRange& range = renderRanges.mRenderRanges[rangeIndex];
    for(size_t graphicalIndex = range.mStart; graphicalIndex < range.mEnd; ++graphicalIndex)
    {
      const GraphicalFrameData& graphicalFrameData = frameData[graphicalIndex];
      VulkanMesh* vulkanMesh = renderer.mMeshMap.FindValue(graphicalFrameData.mMesh, nullptr);
      VulkanShaderMaterial* vulkanShaderMaterial = renderer.mUniqueZilchShaderMaterialMap.FindValue(graphicalFrameData.mZilchShader, nullptr);
      if(vulkanShaderMaterial == nullptr)
        continue;
      
      VulkanShader* vulkanShader = renderer.mZilchShaderMap[graphicalFrameData.mZilchShader];
      const VulkanPipeline* pipeline = range.mPipeline;
      ErrorIf(pipeline == nullptr, "Shouldn't happen");
      vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVulkanPipeline());

      VkBuffer vertexBuffers[] = {vulkanMesh->mVertexBuffer};
      VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, vertexBuffers, offsets);
      vkCmdBindIndexBuffer(vkCommandBuffer, vulkanMesh->mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetVulkanPipelineLayout(), 0, 1, &vulkanShaderMaterial->mDescriptorSets[frameId], dynamicOffsetsCount, dynamicOffsetsBase);
      vkCmdDrawIndexed(vkCommandBuffer, vulkanMesh->mIndexCount, 1, 0, 0, 0);

      for(size_t j = 0; j < dynamicOffsetsCount; ++j)
        dynamicOffsetsBase[j] += dynamicOffsets[j];
    }
  }
}

void DrawPass(RendererData& rendererData, const RenderGraph::PhysicalPass& physicalPass)
{
  VulkanRenderer& renderer = *rendererData.mRenderer;
  uint32_t frameId = GetFrameId(rendererData);
  VulkanRuntimeData& runtimeData = *rendererData.mRuntimeData;
  VulkanRenderFrame& vulkanRenderFrame = runtimeData.mRenderFrames[frameId];
  VulkanCommandBuffer* commandBuffer = vulkanRenderFrame.mCommandBuffer;

  PopulateTransformBuffers(renderer, *physicalPass.mViewBlock, physicalPass.mFrameData);

  Array<VkClearValue> clearValues;
  clearValues.Resize(physicalPass.mClearColors.Size() + physicalPass.mClearDepths.Size());
  for(size_t i = 0; i < physicalPass.mClearColors.Size(); ++i)
  {
    Vec4 color = physicalPass.mClearColors[i].mColor;
    clearValues[i].color = {color.x, color.y, color.z, color.w};
  }
  for(size_t i = 0; i < physicalPass.mClearDepths.Size(); ++i)
  {
    size_t index = i + physicalPass.mClearColors.Size();
    float depth = physicalPass.mClearDepths[i].mDepth;
    uint32_t stencil = physicalPass.mClearDepths[i].mStencil;
    clearValues[index].depthStencil.depth = depth;
    clearValues[index].depthStencil.stencil = stencil;
  }

  VkExtent2D extent = runtimeData.mSwapChain->GetExtent();
  
  VkRenderPassBeginInfo renderPassBeginInfo = {};
  renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassBeginInfo.framebuffer = physicalPass.mFrameBuffer->GetVulkanFrameBuffer();
  renderPassBeginInfo.renderPass = physicalPass.mRenderPass->GetVulkanRenderPass();
  renderPassBeginInfo.renderArea.offset = {0, 0};
  renderPassBeginInfo.renderArea.extent = extent;
  renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.Size());
  renderPassBeginInfo.pClearValues = clearValues.Data();

  VkCommandBuffer vkCommandBuffer = commandBuffer->GetVulkanCommandBuffer();
  commandBuffer->Begin();

  vkCmdBeginRenderPass(vkCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

  AddFrameDataDrawCommands(renderer, *commandBuffer, physicalPass.mRenderRanges, physicalPass.mFrameData);

  commandBuffer->EndRenderPass();
  commandBuffer->End();
}

void ProcessRenderQueue(RendererData& rendererData, const RenderQueue& renderQueue)
{
  GlobalBufferOffset offsets;
  PopulateGlobalBuffers(rendererData, renderQueue, offsets);

  RenderGraphCreationInfo renderGraphCreationInfo;
  renderGraphCreationInfo.mDevice = rendererData.mRuntimeData->mDevice;
  RenderGraph renderGraph(renderGraphCreationInfo);
  renderGraph.Bake(*rendererData.mRenderer, renderQueue);

  for(auto&& physicalPass : renderGraph.mPasses)
  {
    DrawPass(rendererData, physicalPass);
  }
}
