#pragma once

struct CommandBuffersCreationData
{
  VkDevice mDevice;
  VkCommandPool mCommandPool;
  VkRenderPass mRenderPass;
  VkPipeline mGraphicsPipeline;
  VkPipelineLayout mPipelineLayout;
  VkBuffer mVertexBuffer;
  VkBuffer mIndexBuffer;
  VkSwapchainKHR mSwapChain;
  VkExtent2D mSwapChainExtent;
  uint32_t mIndexBufferCount;
  Array<VkFramebuffer> mSwapChainFramebuffers;
  Array<VkDescriptorSet> mDescriptorSets;
};

struct CommandBuffersResultData
{
  Array<VkCommandBuffer> mCommandBuffers;
};

struct CommandBufferWriteInfo
{
  VkDevice mDevice;
  VkCommandPool mCommandPool;
  VkRenderPass mRenderPass;
  VkPipeline mGraphicsPipeline;
  VkPipelineLayout mPipelineLayout;
  VkBuffer mVertexBuffer;
  VkBuffer mIndexBuffer;
  VkSwapchainKHR mSwapChain;
  VkExtent2D mSwapChainExtent;
  uint32_t mIndexBufferCount;
  VkFramebuffer mSwapChainFramebuffer;
  VkDescriptorSet mDescriptorSet;

  uint32_t mDrawCount = 0;
  uint32_t* mDynamicOffsets = nullptr;
  uint32_t* mDynamicOffsetsBase = nullptr;
  uint32_t mDynamicOffsetsCount = 0;
};

inline VulkanStatus BeginCommandBuffer(VkCommandBuffer& commandBuffer)
{
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0; // Optional
  beginInfo.pInheritanceInfo = nullptr; // Optional

  VulkanStatus result;
  if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    result.MarkFailed("failed to begin recording command buffer!");
  return result;
}

inline VulkanStatus EndCommandBuffer(VkCommandBuffer& commandBuffer)
{
  VulkanStatus result;
  if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    result.MarkFailed("failed to record command buffer!");
  return result;
}

inline VulkanStatus BeginRenderPass(CommandBufferWriteInfo& writeInfo, VkCommandBuffer& commandBuffer)
{
  VkRenderPassBeginInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = writeInfo.mRenderPass;
  renderPassInfo.framebuffer = writeInfo.mSwapChainFramebuffer;
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = writeInfo.mSwapChainExtent;
  std::array<VkClearValue, 2> clearValues = {};
  clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
  return VulkanStatus();
}

inline VulkanStatus EndRenderPass(CommandBufferWriteInfo& writeInfo, VkCommandBuffer& commandBuffer)
{
  vkCmdEndRenderPass(commandBuffer);
  return VulkanStatus();
}

inline VulkanStatus WriteCommandBuffer(CommandBufferWriteInfo& writeInfo, VkCommandBuffer& commandBuffer)
{
  BeginCommandBuffer(commandBuffer);

  BeginRenderPass(writeInfo, commandBuffer);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, writeInfo.mGraphicsPipeline);

  VkBuffer vertexBuffers[] = {writeInfo.mVertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer, writeInfo.mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
  

  for(uint32_t i = 0; i < writeInfo.mDrawCount; ++i)
  {
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, writeInfo.mPipelineLayout, 0, 1, &writeInfo.mDescriptorSet, writeInfo.mDynamicOffsetsCount, writeInfo.mDynamicOffsetsBase);
    vkCmdDrawIndexed(commandBuffer, writeInfo.mIndexBufferCount, 1, 0, 0, 0);

    for(size_t j = 0; j < writeInfo.mDynamicOffsetsCount; ++j)
      writeInfo.mDynamicOffsetsBase[j] += writeInfo.mDynamicOffsets[j];
  }

  EndRenderPass(writeInfo, commandBuffer);

  EndCommandBuffer(commandBuffer);
  return VulkanStatus();
}

inline VulkanStatus CreateCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* resultBuffers, uint32_t resultBuffersCount)
{
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = resultBuffersCount;

  VulkanStatus result;
  if(vkAllocateCommandBuffers(device, &allocInfo, resultBuffers) != VK_SUCCESS)
    result.MarkFailed("failed to allocate command buffers!");
  return result;
}

inline void CreateCommandBuffers(CommandBuffersCreationData& creationData, CommandBuffersResultData& resultData)
{
  resultData.mCommandBuffers.Resize(creationData.mSwapChainFramebuffers.Size());
  CreateCommandBuffer(creationData.mDevice, creationData.mCommandPool, resultData.mCommandBuffers.Data(), static_cast<uint32_t>(resultData.mCommandBuffers.Size()));

  for(size_t i = 0; i < resultData.mCommandBuffers.Size(); i++)
  {
    CommandBufferWriteInfo writeInfo;
    writeInfo.mDevice = creationData.mDevice;
    writeInfo.mCommandPool = creationData.mCommandPool;
    writeInfo.mRenderPass = creationData.mRenderPass;
    writeInfo.mGraphicsPipeline = creationData.mGraphicsPipeline;
    writeInfo.mPipelineLayout = creationData.mPipelineLayout;
    writeInfo.mVertexBuffer = creationData.mVertexBuffer;
    writeInfo.mIndexBuffer = creationData.mIndexBuffer;
    writeInfo.mSwapChain = creationData.mSwapChain;
    writeInfo.mSwapChainExtent = creationData.mSwapChainExtent;
    writeInfo.mIndexBufferCount = creationData.mIndexBufferCount;
    writeInfo.mSwapChainFramebuffer = creationData.mSwapChainFramebuffers[i];
    writeInfo.mDescriptorSet = creationData.mDescriptorSets[i];
    WriteCommandBuffer(writeInfo, resultData.mCommandBuffers[i]);
  }
}
