#include "Precompiled.hpp"

#include "SimpleRendererComponent.hpp"
#include "RenderTasks.hpp"
#include "Space.hpp"
#include "Engine.hpp"
#include "GraphicsSpace.hpp"
#include "GraphicsEngine.hpp"
#include "GraphicalEntry.hpp"
#include "RenderQueue.hpp"

#include "VulkanStructures.hpp"
#include "VulkanInitialization.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanImageView.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanFrameBuffer.hpp"
#include "VulkanRendering.hpp"
#include "RenderGraph.hpp"

//-------------------------------------------------------------------SimpleRendererComponent
ZilchDefineType(SimpleRendererComponent, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindFieldProperty(mActive);
}

void SimpleRendererComponent::Initialize(const CompositionInitializer& initializer)
{
  Zilch::EventConnect(GetSpace(), Events::CollectRenderTasks, &SimpleRendererComponent::OnCollectRenderTasks, this);
}

void SimpleRendererComponent::OnCollectRenderTasks(RenderTaskEvent* renderTaskEvent)
{
  if(!mActive)
    return;

  GraphicsSpace* graphicsSpace = renderTaskEvent->mGraphicsSpace;
  GraphicsEngine* graphicsEngine = graphicsSpace->mEngine;
  VulkanRenderer& vulkanRenderer = graphicsEngine->mRenderer;
  ResourceSystem* resourceSystem = graphicsEngine->mResourceSystem;
  Engine* engine = GetEngine();

  VulkanRuntimeData& runtimeData = *vulkanRenderer.mInternal;
  uint32_t frameId = runtimeData.mCurrentImageIndex;
  VulkanRenderFrame& vulkanRenderFrame = runtimeData.mRenderFrames[frameId];
  VulkanCommandBuffer* vulkanCommandBuffer = vulkanRenderFrame.mCommandBuffer;
  VkCommandBuffer commandBuffer = vulkanCommandBuffer->GetVulkanCommandBuffer();
  VulkanImage* finalColorImage = runtimeData.mSwapChain->GetImage(frameId);
  VulkanImage* finalDepthImage = runtimeData.mDepthImage;

  Array<GraphicalFrameData> frameData;
  CollectFrameData(graphicsSpace, frameData);
  UploadBuffers(vulkanRenderer, *renderTaskEvent->mViewBlock, frameData);

  VulkanImage* colorImage = runtimeData.mSwapChain->GetImage(frameId);
  VkClearColorValue clearColors = {1, 0, 0, 1};
  
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0; // Optional
  beginInfo.pInheritanceInfo = nullptr; // Optional

  VulkanImageViewInfo colorInfo;
  colorInfo.mFormat = runtimeData.mSwapChain->GetImageFormat();
  colorInfo.mAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
  VulkanImageView* colorImageView = new VulkanImageView(runtimeData.mDevice, finalColorImage, colorInfo);

  VulkanImageViewInfo depthInfo;
  depthInfo.mFormat = runtimeData.mDepthFormat;
  depthInfo.mAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
  VulkanImageView* depthImageView = new VulkanImageView(runtimeData.mDevice, finalDepthImage, depthInfo);
  
  VulkanRenderPassInfo renderPassInfo;
  renderPassInfo.mColorAttachments[0] = colorImageView;
  renderPassInfo.mColorDescriptions[0].mInitialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  //renderPassInfo.mColorDescriptions[0].mInitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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
  VulkanRenderPass* renderPass = runtimeData.mRenderPassCache->FindOrCreate(renderPassInfo);

  VkExtent2D extent = runtimeData.mSwapChain->GetExtent();
  VulkanFrameBuffer* frameBuffer = new VulkanFrameBuffer(runtimeData.mDevice, *renderPass, renderPassInfo, extent.width, extent.height);

  vulkanRenderFrame.mResources.Add(colorImageView);
  vulkanRenderFrame.mResources.Add(depthImageView);
  vulkanRenderFrame.mResources.Add(frameBuffer);

  VkRenderPassBeginInfo renderPassBeginInfo = {};
  renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassBeginInfo.renderPass = renderPass->GetVulkanRenderPass();
  renderPassBeginInfo.framebuffer = frameBuffer->GetVulkanFrameBuffer();
  renderPassBeginInfo.renderArea.offset = {0, 0};
  renderPassBeginInfo.renderArea.extent = runtimeData.mSwapChain->GetExtent();
  std::array<VkClearValue, 2> clearValues = {};
  clearValues[0].color = {0.0f, 1.0f, 0.0f, 0.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassBeginInfo.pClearValues = clearValues.data();

  vulkanCommandBuffer->Begin();

  vulkanCommandBuffer->ImageBarrier(*colorImageView, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
  vulkanCommandBuffer->ClearColorImage(*finalColorImage, clearColors, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

  vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

  AddGraphicalDrawCommands(vulkanRenderer, *renderPass, *vulkanCommandBuffer, frameData);

  vulkanCommandBuffer->EndRenderPass();
  vulkanCommandBuffer->End();
}

void SimpleRendererComponent::CollectFrameData(GraphicsSpace* graphicsSpace, Array<GraphicalFrameData>& frameData)
{
  GraphicsEngine* graphicsEngine = graphicsSpace->mEngine;
  ResourceSystem* resourceSystem = graphicsEngine->mResourceSystem;

  RenderGroupManager* renderGroupManager = resourceSystem->FindResourceManager(RenderGroupManager);
  RenderGroup* opaqueRenderGroup = renderGroupManager->FindResource(ResourceName{"Opaque"});
  RenderGroupRenderTask tempRenderGroupTask;
  tempRenderGroupTask.AddRenderGroup(opaqueRenderGroup);
  Array<GraphicalEntry> graphicalEntries;
  graphicsSpace->CollectRenderGroups(&tempRenderGroupTask, graphicalEntries);

  frameData.Resize(graphicalEntries.Size());
  for(size_t i = 0; i < graphicalEntries.Size(); ++i)
  {
    graphicalEntries[i].mGraphical->FilloutFrameData(frameData[i]);
  }
}

void SimpleRendererComponent::UploadBuffers(VulkanRenderer& renderer, ViewBlock& viewBlock, Array<GraphicalFrameData>& frameData)
{
  PopulateTransformBuffers(renderer, viewBlock, frameData);
}

void SimpleRendererComponent::AddGraphicalDrawCommands(VulkanRenderer& renderer, VulkanRenderPass& renderPass, VulkanCommandBuffer& commandBuffer, Array<GraphicalFrameData>& frameData)
{
  RenderPipelineSettings pipelineSettings;
  RenderRanges renderRanges;
  renderRanges.Build(renderer, pipelineSettings, renderPass, frameData);
  AddFrameDataDrawCommands(renderer, commandBuffer, renderRanges, frameData);
}
