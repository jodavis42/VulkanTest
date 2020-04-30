#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

#include "Utilities/File.hpp"
#include "Graphics/Vertex.hpp"

#include "VulkanValidationLayers.hpp"
#include "VulkanPhysicsDeviceSelection.hpp"
#include "VulkanLogicalDeviceCreation.hpp"
#include "VulkanBufferCreation.hpp"
#include "VulkanStructures.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanStatus.hpp"
#include "VulkanSyncronization.hpp"
#include "VulkanRendererInit.hpp"
#include "VulkanSwapChain.hpp"

struct ConstantSwapChainInfo
{
  VkSurfaceFormatKHR mFormat;
};

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

  static constexpr size_t mMaxFramesInFlight = 2;
  VkInstance mInstance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
  SurfaceCreationDelegate mSurfaceCreationCallback;
  VkSurfaceKHR mSurface;
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  PhysicalDeviceLimits mDeviceLimits;
  VkDevice mDevice = VK_NULL_HANDLE;
  VkCommandPool mCommandPool;
  SyncObjects mSyncObjects;

  
  VulkanImage mDepthImage;
  VkFormat mDepthFormat;
  uint32_t mWidth;
  uint32_t mHeight;
  bool mResized = false;

  ConstantSwapChainInfo mSwapChainInfo;
  SwapChainData mSwapChain;

  
  std::vector<VulkanRenderFrame> mRenderFrames;
  
  
  
  VkQueue mGraphicsQueue;
  
  VkQueue mPresentQueue;
  VkRenderPass mRenderPass;
  VkPipelineLayout mPipelineLayout;
  VkPipeline mGraphicsPipeline;
  

  std::vector<VkFramebuffer> mSwapChainFramebuffers;
  std::vector<VkCommandBuffer> mCommandBuffers;

  
  uint32_t mCurrentFrame = 0;

  bool mFramebufferResized = false;
  VkBuffer mVertexBuffer;
  VkDeviceMemory mVertexBufferMemory;

  VkBuffer mIndexBuffer;
  VkDeviceMemory mIndexBufferMemory;

  
  std::unordered_map <uint32_t, VulkanUniformBuffers> mUniformBufferMap;
  uint32_t mLastUsedMaterialBufferId = 0;
  std::unordered_map<uint32_t, VulkanUniformBuffer> mMaterialBuffers;
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

inline void CreateSurface(VkInstance instance, SurfaceCreationDelegate callback, VkSurfaceKHR& outSurface)
{
  if(callback.mUserData != nullptr)
    callback.mCallbackFn(instance, callback.mUserData, outSurface);
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
  QueryPhysicalDeviceLimits(resultData.mPhysicalDevice, runtimeData.mDeviceLimits);

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
  creationData.mSwapChainImageFormat = runtimeData.mSwapChain.mImageFormat;
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
  graphicsPipelineData.mViewportOffset = Vec2((float)runtimeData.mSwapChain.mExtent.width, (float)runtimeData.mSwapChain.mExtent.height);
  graphicsPipelineData.mVertexAttributeDescriptions = VulkanVertex::getAttributeDescriptions();
  graphicsPipelineData.mVertexBindingDescriptions = VulkanVertex::getBindingDescription();
  CreateGraphicsPipeline(graphicsPipelineData);

  runtimeData.mGraphicsPipeline = graphicsPipelineData.mGraphicsPipeline;
  runtimeData.mPipelineLayout = graphicsPipelineData.mPipelineLayout;
}

inline void CreateVertexBuffer(VulkanRuntimeData& runtimeData)
{
  VulkanBufferCreationData vulkanData{runtimeData.mPhysicalDevice, runtimeData.mDevice, runtimeData.mGraphicsQueue, runtimeData.mCommandPool};
  VkDeviceSize bufferSize = sizeof(runtimeData.mVertices[0]) * runtimeData.mVertices.size();
  CreateBuffer(vulkanData, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, runtimeData.mVertexBuffer, runtimeData.mVertexBufferMemory, runtimeData.mVertices.data(), bufferSize);
}

inline void CreateIndexBuffer(VulkanRuntimeData& runtimeData)
{
  VulkanBufferCreationData vulkanData{runtimeData.mPhysicalDevice, runtimeData.mDevice, runtimeData.mGraphicsQueue, runtimeData.mCommandPool};
  VkDeviceSize bufferSize = sizeof(runtimeData.mIndices[0]) * runtimeData.mIndices.size();
  CreateBuffer(vulkanData, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, runtimeData.mIndexBuffer, runtimeData.mIndexBufferMemory, runtimeData.mIndices.data(), bufferSize);
}

inline VulkanStatus CreateCommandPool(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, VkCommandPool& commandPool)
{
  QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice, surface);

  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
  poolInfo.flags = 0; // Optional
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

  VulkanStatus result;
  if(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    result.MarkFailed("failed to create command pool!");
  return result;
}

inline VulkanUniformBuffer* GetUniformBuffer(VulkanRuntimeData* runtimeData, UniformBufferType::Enum bufferType, uint32_t bufferId, uint32_t frameIndex)
{
  VulkanUniformBuffer* buffer = nullptr;
  if(bufferType == UniformBufferType::Global)
  {
    if(frameIndex >= runtimeData->mSwapChain.GetCount())
      __debugbreak();
    buffer = &runtimeData->mUniformBufferMap[bufferId].mBuffers[frameIndex];
  }
  else if(bufferType == UniformBufferType::Material)
  {
    buffer = &runtimeData->mMaterialBuffers[bufferId];
  }
  return buffer;
}


inline void InitializeVulkan(VulkanRuntimeData& runtimeData)
{
  CreateInstance(runtimeData.mInstance);
  SetupDebugMessenger(runtimeData.mInstance, runtimeData.mDebugMessenger);
  CreateSurface(runtimeData.mInstance, runtimeData.mSurfaceCreationCallback, runtimeData.mSurface);
  SelectPhysicalDevice(runtimeData);
  CreateLogicalDevice(runtimeData);
  CreateCommandPool(runtimeData.mPhysicalDevice, runtimeData.mDevice, runtimeData.mSurface, runtimeData.mCommandPool);
  CreateSyncObjects(runtimeData.mDevice, VulkanRuntimeData::mMaxFramesInFlight, runtimeData.mSyncObjects);
  //CreateSwapChain(runtimeData);
  //CreateImageViews(runtimeData);
  //CreateRenderPass(runtimeData);
  //CreateGraphicsPipeline(runtimeData);
  //CreateFramebuffers(runtimeData);
  //CreateCommandPool(runtimeData);
  //CreateVertexBuffer(runtimeData);
  //CreateIndexBuffer(runtimeData);
  //CreateCommandBuffers(runtimeData);
  //CreateSyncObjects(runtimeData);


}

