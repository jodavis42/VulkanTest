#include "Precompiled.hpp"

#include "VulkanRendering.hpp"

#include "VulkanRenderer.hpp"
#include "VulkanInitialization.hpp"
#include "VulkanCommandBuffer.hpp"
#include "RenderQueue.hpp"

uint32_t GetFrameId(RendererData& rendererData)
{
  return rendererData.mRuntimeData->mCurrentImageIndex;
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

void PopulateTransformBuffers(RendererData& rendererData, const ViewBlock& viewBlock, const RenderGroupRenderTask& renderGroupTask)
{
  VulkanRenderer& renderer = *rendererData.mRenderer;
  uint32_t frameId = GetFrameId(rendererData);
  TransformData transformData;

  transformData.mPerspectiveToApiPerspective.SetIdentity();
  transformData.mWorldToView = viewBlock.mWorldToView;
  transformData.mViewToPerspective = viewBlock.mViewToPerspective;

  byte* data = static_cast<byte*>(renderer.MapPerFrameUniformBufferMemory(TransformsBufferName, 0, frameId));

  size_t offset = 0;
  size_t objCount = renderGroupTask.mFrameData.Size();
  for(size_t i = 0; i < objCount; ++i)
  {
    const GraphicalFrameData& graphicalFrameData = renderGroupTask.mFrameData[i];
    transformData.mLocalToWorld = graphicalFrameData.mLocalToWorld;
    byte* memory = data + offset + renderer.AlignUniformBufferOffset(sizeof(TransformData)) * i;
    memcpy(memory, &transformData, sizeof(transformData));
  }
  renderer.UnMapPerFrameUniformBufferMemory(TransformsBufferName, 0, frameId);
}

void DrawModels(RendererData& rendererData, const ViewBlock& viewBlock, const RenderGroupRenderTask& renderGroupTask)
{
  VulkanRenderer& renderer = *rendererData.mRenderer;
  VulkanRuntimeData& runtimeData = *rendererData.mRuntimeData;
  uint32_t frameId = GetFrameId(rendererData);
  VulkanRenderFrame& vulkanRenderFrame = runtimeData.mRenderFrames[frameId];
  VkCommandBuffer commandBuffer = vulkanRenderFrame.mCommandBuffer;
  size_t objCount = renderGroupTask.mFrameData.Size();

  uint32_t dynamicOffsets[1] =
  {
    static_cast<uint32_t>(renderer.AlignUniformBufferOffset(sizeof(TransformData)))
  };

  uint32_t baseOffset = 0;
  uint32_t dynamicOffsetBase[1] = {baseOffset};
  CommandBufferWriteInfo writeInfo;
  writeInfo.mDevice = renderer.mInternal->mDevice;
  writeInfo.mCommandPool = renderer.mInternal->mCommandPool;
  writeInfo.mRenderPass = renderer.mInternal->mRenderFrames[0].mRenderPass;
  writeInfo.mSwapChain = renderer.mInternal->mSwapChain.mSwapChain;
  writeInfo.mSwapChainExtent = renderer.mInternal->mSwapChain.mExtent;
  writeInfo.mSwapChainFramebuffer = vulkanRenderFrame.mFrameBuffer;
  writeInfo.mDrawCount = static_cast<uint32_t>(objCount);
  writeInfo.mDynamicOffsetsCount = 1;
  writeInfo.mDynamicOffsetsBase = dynamicOffsetBase;
  writeInfo.mDynamicOffsets = dynamicOffsets;

  BeginCommandBuffer(commandBuffer);
  BeginRenderPass(writeInfo, commandBuffer);

  for(size_t i = 0; i < objCount; ++i)
  {
    const GraphicalFrameData& graphicalFrameData = renderGroupTask.mFrameData[i];
    VulkanMesh* vulkanMesh = renderer.mMeshMap.FindValue(graphicalFrameData.mMesh, nullptr);
    VulkanShaderMaterial* vulkanShaderMaterial = renderer.mUniqueZilchShaderMaterialMap.FindValue(graphicalFrameData.mZilchShader, nullptr);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanShaderMaterial->mPipeline);

    VkBuffer vertexBuffers[] = {vulkanMesh->mVertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, vulkanMesh->mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanShaderMaterial->mPipelineLayout, 0, 1, &vulkanShaderMaterial->mDescriptorSets[frameId], writeInfo.mDynamicOffsetsCount, writeInfo.mDynamicOffsetsBase);
    vkCmdDrawIndexed(commandBuffer, vulkanMesh->mIndexCount, 1, 0, 0, 0);

    for(size_t j = 0; j < writeInfo.mDynamicOffsetsCount; ++j)
      writeInfo.mDynamicOffsetsBase[j] += writeInfo.mDynamicOffsets[j];
  }

  EndRenderPass(writeInfo, commandBuffer);
  EndCommandBuffer(commandBuffer);
}

void ProcessRenderQueue(RendererData& rendererData, const RenderQueue& renderQueue)
{
  GlobalBufferOffset offsets;
  PopulateGlobalBuffers(rendererData, renderQueue, offsets);
  for(const ViewBlock& viewBlock : renderQueue.mViewBlocks)
  {
    const FrameBlock& frameBlock = renderQueue.mFrameBlocks[viewBlock.mFrameBlockId];
    for(const RenderTask* task : viewBlock.mRenderTaskEvent.mRenderTasks)
    {
      if(task->mTaskType == RenderTaskType::RenderGroup)
      {
        const RenderGroupRenderTask* renderGroupTask = reinterpret_cast<const RenderGroupRenderTask*>(task);
        PopulateTransformBuffers(rendererData, viewBlock, *renderGroupTask);
        DrawModels(rendererData, viewBlock, *renderGroupTask);
      }
    }
  }
}
