#include "Precompiled.hpp"

#include "VulkanRendering.hpp"

#include "VulkanRenderer.hpp"
#include "VulkanInitialization.hpp"
#include "VulkanCommandBuffer.hpp"

uint32_t GetFrameId(RendererData& rendererData)
{
  return rendererData.mRenderer->mCurrentFrame->mId;
}

void PopulateGlobalBuffers(RendererData& rendererData, VulkanGlobalBufferData& globalBufferData)
{
  VulkanRenderer& renderer = *rendererData.mRenderer;
  uint32_t frameId = GetFrameId(rendererData);
  const FrameData& frameData = *globalBufferData.mFrameData;
  const CameraData& cameraData = *globalBufferData.mCameraData;

  size_t offset = 0;
  byte* data = static_cast<byte*>(renderer.MapPerFrameUniformBufferMemory(GlobalsBufferName, 0, frameId));
  memcpy(data, &frameData, sizeof(frameData));
  offset += renderer.AlignUniformBufferOffset(sizeof(frameData));
  memcpy(data, &frameData, sizeof(cameraData));
  offset += renderer.AlignUniformBufferOffset(sizeof(cameraData));
  renderer.UnMapPerFrameUniformBufferMemory(GlobalsBufferName, 0, frameId);
}

void PopulateTransformBuffers(RendererData& rendererData, VulkanTransformBufferData& modelRenderData)
{
  VulkanRenderer& renderer = *rendererData.mRenderer;
  uint32_t frameId = GetFrameId(rendererData);
  TransformData transformData;

  transformData.mPerspectiveToApiPerspective.SetIdentity();
  transformData.mWorldToView = modelRenderData.mWorldToView;
  transformData.mViewToPerspective = modelRenderData.mViewToPerspective;

  byte* data = static_cast<byte*>(renderer.MapPerFrameUniformBufferMemory(TransformsBufferName, 0, frameId));

  size_t offset = 0;

  const Array<ModelRenderData>& modelData = *modelRenderData.mModelRenderData;
  size_t count = modelData.Size();
  for(size_t i = 0; i < count; ++i)
  {
    const ModelRenderData& modelRenderData = modelData[i];
    transformData.mLocalToWorld = modelRenderData.mTransform;
    byte* memory = data + offset + renderer.AlignUniformBufferOffset(sizeof(TransformData)) * i;
    memcpy(memory, &transformData, sizeof(transformData));
  }
  renderer.UnMapPerFrameUniformBufferMemory(TransformsBufferName, 0, frameId);
}

void DrawModels(RendererData& rendererData, VulkanTransformBufferData& modelRenderData)
{
  VulkanRenderer& renderer = *rendererData.mRenderer;
  VulkanRuntimeData& runtimeData = *rendererData.mRuntimeData;
  uint32_t frameId = GetFrameId(rendererData);
  VulkanRenderFrame& vulkanRenderFrame = runtimeData.mRenderFrames[frameId];
  VkCommandBuffer commandBuffer = vulkanRenderFrame.mCommandBuffer;
  const Array<ModelRenderData>& modelData = *modelRenderData.mModelRenderData;

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
  writeInfo.mDrawCount = static_cast<uint32_t>(modelData.Size());
  writeInfo.mDynamicOffsetsCount = 1;
  writeInfo.mDynamicOffsetsBase = dynamicOffsetBase;
  writeInfo.mDynamicOffsets = dynamicOffsets;

  BeginCommandBuffer(commandBuffer);
  BeginRenderPass(writeInfo, commandBuffer);

  for(size_t i = 0; i < modelData.Size(); ++i)
  {
    const ModelRenderData& modelRenderData = modelData[i];
    VulkanMesh* vulkanMesh = renderer.mMeshMap.FindValue(modelRenderData.mMesh, nullptr);
    VulkanShaderMaterial* vulkanShaderMaterial = renderer.mUniqueZilchShaderMaterialMap.FindValue(modelRenderData.mZilchShader, nullptr);

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
