#pragma once

#include "Math.hpp"

struct GraphicsPipelineCreationInfo
{
  VkDevice mDevice;
  Vec2 mViewportOffset = Vec2(0, 0);
  Vec2 mViewportSize = Vec2(0, 0);
  VkRenderPass mRenderPass;

  Array<VkVertexInputBindingDescription> mVertexBindingDescriptions;
  Array<VkVertexInputAttributeDescription> mVertexAttributeDescriptions;

  VkShaderModule mVertexShaderModule;
  VkShaderModule mPixelShaderModule;
  String mVertexShaderMainFnName = "main";
  String mPixelShaderMainFnName = "main";
  VkPipelineLayout mPipelineLayout;
};

struct GraphicsPipelineData
{
  VkDevice mDevice;
  Vec2 mViewportOffset;
  Vec2 mViewportSize;
  VkRenderPass mRenderPass;

  Array<char> mVertexShaderCode;
  Array<char> mPixelShaderCode;

  Array<VkVertexInputBindingDescription> mVertexBindingDescriptions;
  Array<VkVertexInputAttributeDescription> mVertexAttributeDescriptions;

  VkPipelineLayout mPipelineLayout;
  VkPipeline mGraphicsPipeline;
  VkDescriptorSetLayout mDescriptorSetLayout;
};

inline VkShaderModule CreateShaderModule(VkDevice& device, const Array<char>& code)
{
  VkShaderModuleCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.Size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.Data());

  VkShaderModule shaderModule;
  if(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    throw std::runtime_error("failed to create shader module!");

  return shaderModule;
}

inline VulkanStatus CreatePipelineLayout(VkDevice device, VkDescriptorSetLayout* descriptorSetLayouts, uint32_t layoutCount, VkPipelineLayout& pipelineLayout)
{
  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = layoutCount;
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
  pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

  VulkanStatus result;
  if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    result.MarkFailed("failed to create pipeline layout!");
  return result;
}

inline VulkanStatus CreatePipelineLayout(VkDevice device, Array<VkDescriptorSetLayout>& descriptorSetLayouts, VkPipelineLayout& pipelineLayout)
{
  return CreatePipelineLayout(device, descriptorSetLayouts.Data(), (uint32_t)descriptorSetLayouts.Size(), pipelineLayout);
}

inline VulkanStatus CreateGraphicsPipeline(GraphicsPipelineCreationInfo& creationInfo, VkPipeline& resultPipeline)
{
  VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
  vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertexShaderStageInfo.module = creationInfo.mVertexShaderModule;
  vertexShaderStageInfo.pName = creationInfo.mVertexShaderMainFnName.c_str();

  VkPipelineShaderStageCreateInfo pixelShaderStageInfo = {};
  pixelShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  pixelShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  pixelShaderStageInfo.module = creationInfo.mPixelShaderModule;
  pixelShaderStageInfo.pName = creationInfo.mPixelShaderMainFnName.c_str();

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo, pixelShaderStageInfo};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(creationInfo.mVertexBindingDescriptions.Size());
  vertexInputInfo.pVertexBindingDescriptions = creationInfo.mVertexBindingDescriptions.Data();
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(creationInfo.mVertexAttributeDescriptions.Size());
  vertexInputInfo.pVertexAttributeDescriptions = creationInfo.mVertexAttributeDescriptions.Data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport = {};
  viewport.x = creationInfo.mViewportOffset.x;
  viewport.y = creationInfo.mViewportOffset.y;
  viewport.width = creationInfo.mViewportSize.x;
  viewport.height = creationInfo.mViewportSize.x;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = {0, 0};
  scissor.extent.width = static_cast<uint32_t>(creationInfo.mViewportSize.x);
  scissor.extent.height = static_cast<uint32_t>(creationInfo.mViewportSize.y);

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f; // Optional
  rasterizer.depthBiasClamp = 0.0f; // Optional
  rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f; // Optional
  multisampling.pSampleMask = nullptr; // Optional
  multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
  multisampling.alphaToOneEnable = VK_FALSE; // Optional

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f; // Optional
  colorBlending.blendConstants[1] = 0.0f; // Optional
  colorBlending.blendConstants[2] = 0.0f; // Optional
  colorBlending.blendConstants[3] = 0.0f; // Optional

  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.minDepthBounds = 0.0f; // Optional
  depthStencil.maxDepthBounds = 1.0f; // Optional
  depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.front = {}; // Optional
  depthStencil.back = {}; // Optional

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = nullptr; // Optional
  pipelineInfo.layout = creationInfo.mPipelineLayout;
  pipelineInfo.renderPass = creationInfo.mRenderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipelineInfo.basePipelineIndex = -1; // Optional

  VulkanStatus result;
  if(vkCreateGraphicsPipelines(creationInfo.mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &resultPipeline) != VK_SUCCESS)
    result.MarkFailed("failed to create graphics pipeline!");
  return result;
}

inline void CreateGraphicsPipeline(GraphicsPipelineData& graphicsPipelineData)
{
  VkShaderModule vertexShaderModule = CreateShaderModule(graphicsPipelineData.mDevice, graphicsPipelineData.mVertexShaderCode);
  VkShaderModule pixelShaderModule = CreateShaderModule(graphicsPipelineData.mDevice, graphicsPipelineData.mPixelShaderCode);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  CreatePipelineLayout(graphicsPipelineData.mDevice, &graphicsPipelineData.mDescriptorSetLayout, 1, graphicsPipelineData.mPipelineLayout);

  GraphicsPipelineCreationInfo creationInfo;
  creationInfo.mVertexShaderModule = vertexShaderModule;
  creationInfo.mPixelShaderModule = pixelShaderModule;
  creationInfo.mVertexShaderMainFnName = "main";
  creationInfo.mPixelShaderMainFnName = "main";
  creationInfo.mDevice = graphicsPipelineData.mDevice;
  creationInfo.mPipelineLayout = graphicsPipelineData.mPipelineLayout;
  creationInfo.mRenderPass = graphicsPipelineData.mRenderPass;
  creationInfo.mViewportSize = graphicsPipelineData.mViewportSize;
  creationInfo.mVertexAttributeDescriptions = graphicsPipelineData.mVertexAttributeDescriptions;
  creationInfo.mVertexBindingDescriptions = graphicsPipelineData.mVertexBindingDescriptions;
  CreateGraphicsPipeline(creationInfo, graphicsPipelineData.mGraphicsPipeline);

  vkDestroyShaderModule(graphicsPipelineData.mDevice, pixelShaderModule, nullptr);
  vkDestroyShaderModule(graphicsPipelineData.mDevice, vertexShaderModule, nullptr);
}
