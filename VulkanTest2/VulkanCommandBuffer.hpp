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
  std::vector<VkFramebuffer> mSwapChainFramebuffers;
  std::vector<VkDescriptorSet> mDescriptorSets;
};

struct CommandBuffersResultData
{
  std::vector<VkCommandBuffer> mCommandBuffers;
};

void CreateCommandBuffers(CommandBuffersCreationData& creationData, CommandBuffersResultData& resultData)
{
  resultData.mCommandBuffers.resize(creationData.mSwapChainFramebuffers.size());

  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = creationData.mCommandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)resultData.mCommandBuffers.size();

  if(vkAllocateCommandBuffers(creationData.mDevice, &allocInfo, resultData.mCommandBuffers.data()) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate command buffers!");

  for(size_t i = 0; i < resultData.mCommandBuffers.size(); i++)
  {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if(vkBeginCommandBuffer(resultData.mCommandBuffers[i], &beginInfo) != VK_SUCCESS)
      throw std::runtime_error("failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = creationData.mRenderPass;
    renderPassInfo.framebuffer = creationData.mSwapChainFramebuffers[i];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = creationData.mSwapChainExtent;
    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(resultData.mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(resultData.mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, creationData.mGraphicsPipeline);

    VkBuffer vertexBuffers[] = {creationData.mVertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(resultData.mCommandBuffers[i], 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(resultData.mCommandBuffers[i], creationData.mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdBindDescriptorSets(resultData.mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, creationData.mPipelineLayout, 0, 1, &creationData.mDescriptorSets[i], 0, nullptr);

    vkCmdDrawIndexed(resultData.mCommandBuffers[i], creationData.mIndexBufferCount, 1, 0, 0, 0);

    vkCmdEndRenderPass(resultData.mCommandBuffers[i]);

    if(vkEndCommandBuffer(resultData.mCommandBuffers[i]) != VK_SUCCESS)
      throw std::runtime_error("failed to record command buffer!");
  }
}
