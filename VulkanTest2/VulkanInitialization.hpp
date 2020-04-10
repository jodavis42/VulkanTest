#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

#include "VulkanValidationLayers.hpp"
#include "VulkanPhysicsDeviceSelection.hpp"
#include "VulkanLogicalDeviceCreation.hpp"
#include "VulkanBufferCreation.hpp"
#include "Vertex.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanPipeline.hpp"
#include "File.hpp"
#include "VulkanStatus.hpp"

struct VulkanRuntimeData
{
  const std::vector<const char*> mDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  const std::vector<Vertex> mVertices =
  {
       {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
  };

  const std::vector<uint16_t> mIndices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
  };

  GLFWwindow* mWindow;
  VkInstance mInstance;
  VkDebugUtilsMessengerEXT mDebugMessenger;
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  VkDevice mDevice;
  VkQueue mGraphicsQueue;
  VkSurfaceKHR mSurface;
  VkQueue mPresentQueue;
  VkSwapchainKHR mSwapChain;
  std::vector<VkImage> mSwapChainImages;
  VkFormat mSwapChainImageFormat;
  VkExtent2D mSwapChainExtent;
  std::vector<VkImageView> mSwapChainImageViews;
  VkRenderPass mRenderPass;
  VkPipelineLayout mPipelineLayout;
  VkPipeline mGraphicsPipeline;
  VkCommandPool mCommandPool;

  std::vector<VkFramebuffer> mSwapChainFramebuffers;
  std::vector<VkCommandBuffer> mCommandBuffers;

  std::vector<VkSemaphore> mImageAvailableSemaphores;
  std::vector<VkSemaphore> mRenderFinishedSemaphores;
  std::vector<VkFence> mInFlightFences;
  size_t mCurrentFrame = 0;

  bool mFramebufferResized = false;
  VkBuffer mVertexBuffer;
  VkDeviceMemory mVertexBufferMemory;

  VkBuffer mIndexBuffer;
  VkDeviceMemory mIndexBufferMemory;
};

inline std::vector<const char*> GetRequiredExtensions()
{
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

  if(enableValidationLayers)
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  return extensions;
}

inline VulkanStatus CreateInstance(VkInstance& instance)
{
  if(enableValidationLayers && !CheckValidationLayerSupport())
    return VulkanStatus("validation layers requested, but not available!");

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if(enableValidationLayers)
  {
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
    PopulateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
  }
  else
  {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }
  auto extensions = GetRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  createInfo.enabledLayerCount = 0;

  VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
  if(result != VK_SUCCESS)
    VulkanStatus("failed to create instance!");
  return VulkanStatus();
}

inline void CreateInstance(VulkanRuntimeData& runtimeData)
{
  VulkanStatus status = CreateInstance(runtimeData.mInstance);
}

inline void CreateSurface(VulkanRuntimeData& runtimeData)
{
  auto result = glfwCreateWindowSurface(runtimeData.mInstance, runtimeData.mWindow, nullptr, &runtimeData.mSurface);
  if(result != VK_SUCCESS)
    throw std::runtime_error("failed to create window surface!");
}

inline bool IsDeviceSuitable(VkPhysicalDevice physicalDevice, DeviceSuitabilityData* data)
{
  VulkanRuntimeData* runtimeData = (VulkanRuntimeData*)data->mUserData;
  QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, data->mSurface);

  bool extensionsSupported = CheckDeviceExtensionSupport(physicalDevice, runtimeData->mDeviceExtensions);

  bool swapChainAdequate = false;
  if(extensionsSupported)
  {
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, data->mSurface);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

inline void SelectPhysicalDevice(VulkanRuntimeData& runtimeData)
{
  runtimeData.mPhysicalDevice = VK_NULL_HANDLE;

  PhysicsDeviceResultData resultData;
  PhysicsDeviceSelectionData selectionData;
  selectionData.mInstance = runtimeData.mInstance;
  selectionData.mSuitabilityData.mUserData = &runtimeData;
  selectionData.mSuitabilityData.mSurface = runtimeData.mSurface;
  selectionData.mSuitabilityData.mDeviceSuitabilityFn = &IsDeviceSuitable;

  SelectPhysicsDevice(selectionData, resultData);

  runtimeData.mPhysicalDevice = resultData.mPhysicalDevice;
}

inline void CreateLogicalDevice(VulkanRuntimeData& runtimeData)
{
  LogicalDeviceResultData resultData;
  LogicalDeviceCreationData creationData;
  creationData.mPhysicalDevice = runtimeData.mPhysicalDevice;
  creationData.mSurface = runtimeData.mSurface;
  creationData.mDeviceExtensions = runtimeData.mDeviceExtensions;

  CreateLogicalDevice(creationData, resultData);

  runtimeData.mDevice = resultData.mDevice;
  runtimeData.mGraphicsQueue = resultData.mGraphicsQueue;
  runtimeData.mPresentQueue = resultData.mPresentQueue;
}

inline void CreateRenderPass(VulkanRuntimeData& runtimeData)
{
  RenderPassCreationData creationData;
  creationData.mRenderPass = runtimeData.mRenderPass;
  creationData.mDevice = runtimeData.mDevice;
  creationData.mSwapChainImageFormat = runtimeData.mSwapChainImageFormat;
  CreateRenderPass(creationData);

  runtimeData.mRenderPass = creationData.mRenderPass;
}

inline void CreateGraphicsPipeline(VulkanRuntimeData& runtimeData)
{
  auto vertexShaderCode = readFile("shaders/vertex.spv");
  auto pixelShaderCode = readFile("shaders/pixel.spv");

  GraphicsPipelineData graphicsPipelineData;
  graphicsPipelineData.mDevice = runtimeData.mDevice;
  graphicsPipelineData.mGraphicsPipeline = runtimeData.mGraphicsPipeline;
  graphicsPipelineData.mPipelineLayout = runtimeData.mPipelineLayout;
  graphicsPipelineData.mPixelShaderCode = pixelShaderCode;
  graphicsPipelineData.mVertexShaderCode = vertexShaderCode;
  graphicsPipelineData.mRenderPass = runtimeData.mRenderPass;
  graphicsPipelineData.mSwapChainExtent = runtimeData.mSwapChainExtent;
  graphicsPipelineData.mVertexAttributeDescriptions = Vertex::getAttributeDescriptions();
  graphicsPipelineData.mVertexBindingDescriptions = Vertex::getBindingDescription();
  CreateGraphicsPipeline(graphicsPipelineData);

  runtimeData.mGraphicsPipeline = graphicsPipelineData.mGraphicsPipeline;
  runtimeData.mPipelineLayout = graphicsPipelineData.mPipelineLayout;
}

inline void CreateVertexBuffer(VulkanRuntimeData& runtimeData)
{
  VulkanBufferCreationData vulkanData{runtimeData.mPhysicalDevice, runtimeData.mDevice, runtimeData.mGraphicsQueue, runtimeData.mGraphicsPipeline, runtimeData.mCommandPool};
  VkDeviceSize bufferSize = sizeof(runtimeData.mVertices[0]) * runtimeData.mVertices.size();
  CreateBuffer(vulkanData, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, runtimeData.mVertexBuffer, runtimeData.mVertexBufferMemory, runtimeData.mVertices.data(), bufferSize);
}

inline void CreateIndexBuffer(VulkanRuntimeData& runtimeData)
{
  VulkanBufferCreationData vulkanData{runtimeData.mPhysicalDevice, runtimeData.mDevice, runtimeData.mGraphicsQueue, runtimeData.mGraphicsPipeline, runtimeData.mCommandPool};
  VkDeviceSize bufferSize = sizeof(runtimeData.mIndices[0]) * runtimeData.mIndices.size();
  CreateBuffer(vulkanData, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, runtimeData.mIndexBuffer, runtimeData.mIndexBufferMemory, runtimeData.mIndices.data(), bufferSize);
}

inline void InitializeVulkan(VulkanRuntimeData& runtimeData)
{
  CreateInstance(runtimeData);
  SetupDebugMessenger(runtimeData.mInstance, runtimeData.mDebugMessenger);
  CreateSurface(runtimeData);
  SelectPhysicalDevice(runtimeData);
  CreateLogicalDevice(runtimeData);
  //CreateSwapChain(runtimeData);
  //CreateImageViews(runtimeData);
  CreateRenderPass(runtimeData);
  CreateGraphicsPipeline(runtimeData);
  //CreateFramebuffers(runtimeData);
  //CreateCommandPool(runtimeData);
  CreateVertexBuffer(runtimeData);
  CreateIndexBuffer(runtimeData);
  //CreateCommandBuffers(runtimeData);
  //CreateSyncObjects(runtimeData);
}

