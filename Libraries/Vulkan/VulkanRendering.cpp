#include "Precompiled.hpp"

#include "VulkanRendering.hpp"

#include "VulkanRenderer.hpp"
#include "VulkanInitialization.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanFrameBuffer.hpp"
#include "VulkanImageView.hpp"
#include "RenderQueue.hpp"

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

void PopulateTransformBuffers(RendererData& rendererData, const ViewBlock& viewBlock, const RenderGroupRenderTask& renderGroupTask)
{
  PopulateTransformBuffers(*rendererData.mRenderer, viewBlock, renderGroupTask.mFrameData);
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

void AddFrameDataDrawCommands(VulkanRenderer& renderer, VulkanCommandBuffer& commandBuffer, const Array<GraphicalFrameData>& frameData)
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
  for(size_t i = 0; i < frameData.Size(); ++i)
  {
    const GraphicalFrameData& graphicalFrameData = frameData[i];
    VulkanMesh* vulkanMesh = renderer.mMeshMap.FindValue(graphicalFrameData.mMesh, nullptr);
    VulkanShaderMaterial* vulkanShaderMaterial = renderer.mUniqueZilchShaderMaterialMap.FindValue(graphicalFrameData.mZilchShader, nullptr);
    if(vulkanShaderMaterial != nullptr)
    {
      vkCmdBindPipeline(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanShaderMaterial->mPipeline);

      VkBuffer vertexBuffers[] = {vulkanMesh->mVertexBuffer};
      VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(vkCommandBuffer, 0, 1, vertexBuffers, offsets);
      vkCmdBindIndexBuffer(vkCommandBuffer, vulkanMesh->mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanShaderMaterial->mPipelineLayout, 0, 1, &vulkanShaderMaterial->mDescriptorSets[frameId], dynamicOffsetsCount, dynamicOffsetsBase);
      vkCmdDrawIndexed(vkCommandBuffer, vulkanMesh->mIndexCount, 1, 0, 0, 0);
    }

    for(size_t j = 0; j < dynamicOffsetsCount; ++j)
      dynamicOffsetsBase[j] += dynamicOffsets[j];
  }
}

void DrawModels(RendererData& rendererData, const ViewBlock& viewBlock, const RenderGroupRenderTask& renderGroupTask)
{
  VulkanRenderer& renderer = *rendererData.mRenderer;
  VulkanRuntimeData& runtimeData = *rendererData.mRuntimeData;
  uint32_t frameId = GetFrameId(rendererData);
  VulkanRenderFrame& vulkanRenderFrame = runtimeData.mRenderFrames[frameId];
  VulkanImage* finalColorImage = runtimeData.mSwapChain->GetImage(frameId);
  VulkanImage* finalDepthImage = runtimeData.mDepthImage;
  VulkanCommandBuffer* commandBuffer = vulkanRenderFrame.mCommandBuffer;

  VulkanImageViewInfo colorImageViewInfo;
  colorImageViewInfo.mFormat = runtimeData.mSwapChain->GetImageFormat();
  colorImageViewInfo.mAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
  VulkanImageView* colorImageView = new VulkanImageView(runtimeData.mDevice, finalColorImage, colorImageViewInfo);

  VulkanImageViewInfo depthImageViewInfo;
  depthImageViewInfo.mFormat = runtimeData.mDepthFormat;
  depthImageViewInfo.mAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
  VulkanImageView* depthImageView = new VulkanImageView(runtimeData.mDevice, finalDepthImage, depthImageViewInfo);

  vulkanRenderFrame.mResources.Add(colorImageView);
  vulkanRenderFrame.mResources.Add(depthImageView);

  VulkanRenderPassInfo renderPassInfo;
  renderPassInfo.mColorAttachments[0] = colorImageView;
  //renderPassInfo.mColorDescriptions[0].mInitialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  renderPassInfo.mColorDescriptions[0].mInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  renderPassInfo.mColorDescriptions[0].mFinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  renderPassInfo.mColorDescriptions[0].mLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  //renderPassInfo.mColorDescriptions[0].mLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  renderPassInfo.mDepthAttachment = depthImageView;
  renderPassInfo.mDepthDescription.mInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  renderPassInfo.mDepthDescription.mFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  renderPassInfo.mColorAttachmentCount = 1;
  auto& subPass = renderPassInfo.mSubPasses.PushBack();
  subPass.mColorAttachmentCount = 1;
  subPass.mColorAttachments[0] = 0;

  VulkanRenderPass* renderPass = new VulkanRenderPass(runtimeData.mDevice, renderPassInfo);
  vulkanRenderFrame.mResources.Add(renderPass);

  VkExtent2D extent = runtimeData.mSwapChain->GetExtent();
  VulkanFrameBuffer* frameBuffer = new VulkanFrameBuffer(runtimeData.mDevice, *renderPass, renderPassInfo, extent.width, extent.height);
  vulkanRenderFrame.mResources.Add(frameBuffer);

  std::array<VkClearValue, 2> clearValues = {};
  clearValues[0].color = {0.0f, 1.0f, 0.0f, 0.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  VkRenderPassBeginInfo renderPassBeginInfo = {};
  renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassBeginInfo.renderPass = renderPass->GetVulkanRenderPass();
  renderPassBeginInfo.framebuffer = frameBuffer->GetVulkanFrameBuffer();
  renderPassBeginInfo.renderArea.offset = {0, 0};
  renderPassBeginInfo.renderArea.extent = extent;
  renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassBeginInfo.pClearValues = clearValues.data();

  VkCommandBuffer vkCommandBuffer = commandBuffer->GetVulkanCommandBuffer();
  commandBuffer->Begin();
  vkCmdBeginRenderPass(vkCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

  AddFrameDataDrawCommands(renderer, *commandBuffer, renderGroupTask.mFrameData);

  commandBuffer->EndRenderPass();
  commandBuffer->End();
}

void ProcessRenderQueue(RendererData& rendererData, const RenderQueue& renderQueue)
{
  GlobalBufferOffset offsets;
  PopulateGlobalBuffers(rendererData, renderQueue, offsets);
  for(const ViewBlock& viewBlock : renderQueue.mViewBlocks)
  {
    const FrameBlock& frameBlock = renderQueue.mFrameBlocks[viewBlock.mFrameBlockId];
    for(const RenderTask* task : viewBlock.mRenderTaskEvent->mRenderTasks)
    {
      if(task->mTaskType == RenderTaskType::ClearTarget)
      {
        const ClearTargetRenderTask* clearTargetTask = reinterpret_cast<const ClearTargetRenderTask*>(task);
        //ClearTarget(rendererData, viewBlock, *clearTargetTask);
      }
      else if(task->mTaskType == RenderTaskType::RenderGroup)
      {
        const RenderGroupRenderTask* renderGroupTask = reinterpret_cast<const RenderGroupRenderTask*>(task);
        PopulateTransformBuffers(rendererData, viewBlock, *renderGroupTask);
        DrawModels(rendererData, viewBlock, *renderGroupTask);
      }
    }
  }
}
