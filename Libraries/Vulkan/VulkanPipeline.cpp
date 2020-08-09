#include "Precompiled.hpp"

#include "VulkanPipeline.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanShaders.hpp"
#include "VulkanStatus.hpp"
#include "EnumConversions.hpp"
#include "RenderPipelineSettings.hpp"

//-------------------------------------------------------------------VulkanPipeline
VulkanPipeline::VulkanPipeline(VkDevice device, const VulkanPipelineCreationInfo& creationInfo)
{
  mDevice = device;
  CreateVulkanPipelineLayout(creationInfo);
  CreateVulkanPipeline(creationInfo);
}

VulkanPipeline::~VulkanPipeline()
{
  Free();
}

void VulkanPipeline::Free()
{
  if(mDevice == VK_NULL_HANDLE)
    return;

  vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
  vkDestroyPipeline(mDevice, mPipeline, nullptr);
  mDevice = VK_NULL_HANDLE;
}

VkPipelineLayout VulkanPipeline::GetVulkanPipelineLayout() const
{
  return mPipelineLayout;
}

VkPipeline VulkanPipeline::GetVulkanPipeline() const
{
  return mPipeline;
}

void VulkanPipeline::CreateVulkanPipelineLayout(const VulkanPipelineCreationInfo& creationInfo)
{
  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(creationInfo.mDescriptorSetLayouts.Size());
  pipelineLayoutInfo.pSetLayouts = creationInfo.mDescriptorSetLayouts.Data();
  pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

  VulkanStatus result;
  if(vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
    result.MarkFailed("failed to create pipeline layout!");
}

void VulkanPipeline::CreateVulkanPipeline(const VulkanPipelineCreationInfo& creationInfo)
{
  VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
  vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertexShaderStageInfo.module = creationInfo.mShader->mVertexShaderModule;
  vertexShaderStageInfo.pName = creationInfo.mShader->mVertexEntryPointName.c_str();

  VkPipelineShaderStageCreateInfo pixelShaderStageInfo = {};
  pixelShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  pixelShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  pixelShaderStageInfo.module = creationInfo.mShader->mPixelShaderModule;
  pixelShaderStageInfo.pName = creationInfo.mShader->mPixelEntryPointName.c_str();

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

  auto&& pipelineSettings = *creationInfo.mPipelineSettings;
  auto&& blendSettings = pipelineSettings.mBlendSettings;
  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = pipelineSettings.mBlendSettings.mBlendMode == BlendMode::Enabled ? VK_TRUE : VK_FALSE;
  colorBlendAttachment.srcColorBlendFactor = ConvertBlendFactor(blendSettings.mColorSourceFactor);
  colorBlendAttachment.dstColorBlendFactor = ConvertBlendFactor(blendSettings.mColorDestFactor);
  colorBlendAttachment.colorBlendOp = ConvertBlendEquation(blendSettings.mColorBlendEquation);
  colorBlendAttachment.srcAlphaBlendFactor = ConvertBlendFactor(blendSettings.mAlphaSourceFactor);
  colorBlendAttachment.dstAlphaBlendFactor = ConvertBlendFactor(blendSettings.mAlphaDestFactor);
  colorBlendAttachment.alphaBlendOp = ConvertBlendEquation(blendSettings.mAlphaBlendEquation);

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

  auto&& depthSettings = pipelineSettings.mDepthSettings;
  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = IsDepthReadEnabled(depthSettings.mDepthMode);
  depthStencil.depthWriteEnable = IsDepthWriteEnabled(depthSettings.mDepthMode);
  depthStencil.depthCompareOp = ConvertTextureCompareOp(depthSettings.mDepthCompareFunc);
  depthStencil.depthBoundsTestEnable = depthSettings.mDepthBoundsTestEnable;
  depthStencil.minDepthBounds = depthSettings.mMinDepthBounds;
  depthStencil.maxDepthBounds = depthSettings.mMaxDepthBounds;
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
  pipelineInfo.layout = mPipelineLayout;
  pipelineInfo.renderPass = creationInfo.mRenderPass->GetVulkanRenderPass();
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
  pipelineInfo.basePipelineIndex = -1; // Optional

  VulkanStatus result;
  if(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline) != VK_SUCCESS)
    result.MarkFailed("failed to create graphics pipeline!");
}
