#include "pch.h"

#include "VulkanRenderer.hpp"

#include "VulkanInitialization.hpp"
#include "VulkanValidationLayers.hpp"

VulkanRenderer::VulkanRenderer()
{
  mInternal = new VulkanRuntimeData();
}

VulkanRenderer::~VulkanRenderer()
{
  Destroy();
}

void VulkanRenderer::Initialize()
{
  InitializeVulkan(*mInternal);
}

void VulkanRenderer::Destroy()
{
  delete mInternal;
}

void VulkanRenderer::Draw()
{

}
