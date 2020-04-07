#include "pch.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
//#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>
#include <array>
#include "File.hpp"
#include "VulkanExtensions.hpp"
#include "VulkanBufferCreation.hpp"
#include "VulkanValidationLayers.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanRenderPass.hpp"
#include "Vertex.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanPhysicsDeviceSelection.hpp"
#include "VulkanDeviceQueries.hpp"
#include "VulkanLogicalDeviceCreation.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

const int cWidth = 800;
const int cHeight = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;
const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct UniformBufferObject {
  alignas(16) glm::mat4 model;
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

class HelloTriangleApplication {
public:
  void run()
  {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
  const std::vector<Vertex> vertices =
  {
      {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
      {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
  };

  const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};

  void initWindow()
  {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(cWidth, cHeight, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
  }

  static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
  {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->mFramebufferResized = true;
  }

  void initVulkan() {
    CreateInstance();
    SetupDebugMessenger(mInstance, mDebugMessenger);
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createTextureImage();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSyncObjects();
  }

  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
  {
    return QuerySwapChainSupport(device, mSurface);
  }

  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for(const auto& availablePresentMode : availablePresentModes)
    {
      if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        return availablePresentMode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
  {
    for(const auto& availableFormat : availableFormats)
    {
      if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        return availableFormat;
    }

    return availableFormats[0];
  }

  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
  {
    if(capabilities.currentExtent.width != UINT32_MAX)
    {
      return capabilities.currentExtent;
    }
    else
    {
      int width, height;
      glfwGetFramebufferSize(mWindow, &width, &height);

      VkExtent2D actualExtent =
      {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
      };

      return actualExtent;
    }
  }

  static bool IsDeviceSuitable(VkPhysicalDevice physicalDevice, DeviceSuitabilityData* data)
  {
    HelloTriangleApplication* self = (HelloTriangleApplication*)data->mUserData;
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, data->mSurface);

    bool extensionsSupported = CheckDeviceExtensionSupport(physicalDevice, deviceExtensions);

    bool swapChainAdequate = false;
    if(extensionsSupported)
    {
      SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, data->mSurface);
      swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
  }

  void createSurface()
  {
    auto result = glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface);
    if(result != VK_SUCCESS)
      throw std::runtime_error("failed to create window surface!");
  }

  void pickPhysicalDevice()
  {
    mPhysicalDevice = VK_NULL_HANDLE;

    PhysicsDeviceResultData resultData;
    PhysicsDeviceSelectionData selectionData;
    selectionData.mInstance = mInstance;
    selectionData.mSuitabilityData.mUserData = this;
    selectionData.mSuitabilityData.mSurface = mSurface;
    selectionData.mSuitabilityData.mDeviceSuitabilityFn = &IsDeviceSuitable;

    SelectPhysicsDevice(selectionData, resultData);
    mPhysicalDevice = resultData.mPhysicalDevice;
  }

  void createLogicalDevice()
  {
    LogicalDeviceResultData resultData;
    LogicalDeviceCreationData creationData;
    creationData.mPhysicalDevice = mPhysicalDevice;
    creationData.mSurface = mSurface;
    creationData.mDeviceExtensions = deviceExtensions;

    CreateLogicalDevice(creationData, resultData);

    mDevice = resultData.mDevice;
    mGraphicsQueue = resultData.mGraphicsQueue;
    mPresentQueue = resultData.mPresentQueue;
  }

  void cleanupSwapChain()
  {
    for(auto framebuffer : mSwapChainFramebuffers)
      vkDestroyFramebuffer(mDevice, framebuffer, nullptr);

    vkFreeCommandBuffers(mDevice, mCommandPool, static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());

    vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
    vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

    for(auto imageView : mSwapChainImageViews)
      vkDestroyImageView(mDevice, imageView, nullptr);

    vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);

    for(size_t i = 0; i < mSwapChainImages.size(); i++)
    {
      vkDestroyBuffer(mDevice, mUniformBuffers[i], nullptr);
      vkFreeMemory(mDevice, mUniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
  }

  void recreateSwapChain()
  {
    int width = 0, height = 0;
    while(width == 0 || height == 0)
    {
      glfwGetFramebufferSize(mWindow, &width, &height);
      glfwWaitEvents();
    }

    vkDeviceWaitIdle(mDevice);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
  }

  void createSwapChain()
  {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(mPhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
      imageCount = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice, mSurface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if(indices.graphicsFamily != indices.presentFamily)
    {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0; // Optional
      createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if(vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
      throw std::runtime_error("failed to create swap chain!");

    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
    mSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, mSwapChainImages.data());

    mSwapChainImageFormat = surfaceFormat.format;
    mSwapChainExtent = extent;
  }

  void createImageViews()
  {
    mSwapChainImageViews.resize(mSwapChainImages.size());

    for(size_t i = 0; i < mSwapChainImages.size(); i++)
    {
      VkImageViewCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = mSwapChainImages[i];
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = mSwapChainImageFormat;
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      if(vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapChainImageViews[i]) != VK_SUCCESS)
        throw std::runtime_error("failed to create image views!");
    }
  }

  void CreateInstance()
  {
    if(enableValidationLayers && !CheckValidationLayerSupport())
      throw std::runtime_error("validation layers requested, but not available!");

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
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    createInfo.enabledLayerCount = 0;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &mInstance);
    if(result != VK_SUCCESS)
      throw std::runtime_error("failed to create instance!");
  }

  std::vector<const char*> getRequiredExtensions()
  {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if(enableValidationLayers)
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
  }

  void createRenderPass()
  {
    RenderPassCreationData creationData;
    creationData.mRenderPass = mRenderPass;
    creationData.mDevice = mDevice;
    creationData.mSwapChainImageFormat = mSwapChainImageFormat;
    CreateRenderPass(creationData);

    mRenderPass = creationData.mRenderPass;
  }

  void createDescriptorSetLayout()
  {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if(vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
      throw std::runtime_error("failed to create descriptor set layout!");
  }

  void createGraphicsPipeline()
  {
    auto vertexShaderCode = readFile("shaders/vertex.spv");
    auto pixelShaderCode = readFile("shaders/pixel.spv");

    GraphicsPipelineData graphicsPipelineData;
    graphicsPipelineData.mDevice = mDevice;
    graphicsPipelineData.mGraphicsPipeline = mGraphicsPipeline;
    graphicsPipelineData.mPipelineLayout = mPipelineLayout;
    graphicsPipelineData.mPixelShaderCode = pixelShaderCode;
    graphicsPipelineData.mVertexShaderCode = vertexShaderCode;
    graphicsPipelineData.mRenderPass = mRenderPass;
    graphicsPipelineData.mSwapChainExtent = mSwapChainExtent;
    graphicsPipelineData.mVertexAttributeDescriptions = Vertex::getAttributeDescriptions();
    graphicsPipelineData.mVertexBindingDescriptions = Vertex::getBindingDescription();
    graphicsPipelineData.mDescriptorSetLayout = mDescriptorSetLayout;
    CreateGraphicsPipeline(graphicsPipelineData);

    mGraphicsPipeline = graphicsPipelineData.mGraphicsPipeline;
    mPipelineLayout = graphicsPipelineData.mPipelineLayout;
  }

  void createFramebuffers()
  {
    mSwapChainFramebuffers.resize(mSwapChainImageViews.size());

    for(size_t i = 0; i < mSwapChainImageViews.size(); i++)
    {
      VkImageView attachments[] = {mSwapChainImageViews[i]};

      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = mRenderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.width = mSwapChainExtent.width;
      framebufferInfo.height = mSwapChainExtent.height;
      framebufferInfo.layers = 1;

      if(vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS)
        throw std::runtime_error("failed to create framebuffer!");
    }
  }

  void createCommandPool()
  {
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(mPhysicalDevice, mSurface);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = 0; // Optional

    if(vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
      throw std::runtime_error("failed to create command pool!");
  }

  void createTextureImage()
  {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("textures/image.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if(!pixels)
      throw std::runtime_error("failed to load texture image!");

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VulkanBufferCreationData vulkanData{mPhysicalDevice, mDevice, mGraphicsQueue, mGraphicsPipeline, mCommandPool};
    CreateBuffer(vulkanData, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(mDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(mDevice, stagingBufferMemory);

    stbi_image_free(pixels);

    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    createImage(texWidth, texHeight, format, tiling, usage, properties, mTextureImage, mTextureImageMemory);

    transitionImageLayout(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, mTextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    transitionImageLayout(mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
    vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
  }

  void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
  {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional

    if(vkCreateImage(mDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
      throw std::runtime_error("failed to create image!");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(mDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

    VulkanBufferCreationData vulkanData{mPhysicalDevice, mDevice, mGraphicsQueue, mGraphicsPipeline, mCommandPool};
    allocInfo.memoryTypeIndex = FindMemoryType(vulkanData, memRequirements.memoryTypeBits, properties);

    if(vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(mDevice, image, imageMemory, 0);
  }

  void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
  {
    VulkanBufferCreationData vulkanData{mPhysicalDevice, mDevice, mGraphicsQueue, mGraphicsPipeline, mCommandPool};

    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(vulkanData);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
      throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
      commandBuffer,
      0 /* TODO */, 0 /* TODO */,
      0,
      0, nullptr,
      0, nullptr,
      1, &barrier
    );

    EndSingleTimeCommands(vulkanData, commandBuffer);
  }

  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
  {
    VulkanBufferCreationData vulkanData{mPhysicalDevice, mDevice, mGraphicsQueue, mGraphicsPipeline, mCommandPool};

    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(vulkanData);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };
    vkCmdCopyBufferToImage(
      commandBuffer,
      buffer,
      image,
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1,
      &region
    );

    EndSingleTimeCommands(vulkanData, commandBuffer);
  }

  void createVertexBuffer()
  {
    VulkanBufferCreationData vulkanData{mPhysicalDevice, mDevice, mGraphicsQueue, mGraphicsPipeline, mCommandPool};
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    CreateBuffer(vulkanData, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mVertexBuffer, mVertexBufferMemory, vertices.data(), bufferSize);
  }

  void createIndexBuffer()
  {
    VulkanBufferCreationData vulkanData{mPhysicalDevice, mDevice, mGraphicsQueue, mGraphicsPipeline, mCommandPool};
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
    CreateBuffer(vulkanData, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mIndexBuffer, mIndexBufferMemory, indices.data(), bufferSize);
  }

  void createUniformBuffers()
  {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    mUniformBuffers.resize(mSwapChainImages.size());
    mUniformBuffersMemory.resize(mSwapChainImages.size());

    VulkanBufferCreationData vulkanData{mPhysicalDevice, mDevice, mGraphicsQueue, mGraphicsPipeline, mCommandPool};

    for(size_t i = 0; i < mSwapChainImages.size(); i++)
    {
      CreateBuffer(vulkanData, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mUniformBuffers[i], mUniformBuffersMemory[i]);
    }
  }

  void createDescriptorPool()
  {
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(mSwapChainImages.size());

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(mSwapChainImages.size());

    if(vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
      throw std::runtime_error("failed to create descriptor pool!");
  }

  void createDescriptorSets()
  {
    uint32_t count = static_cast<uint32_t>(mSwapChainImages.size());
    std::vector<VkDescriptorSetLayout> layouts(count, mDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mDescriptorPool;
    allocInfo.descriptorSetCount = count;
    allocInfo.pSetLayouts = layouts.data();

    mDescriptorSets.resize(count);
    if(vkAllocateDescriptorSets(mDevice, &allocInfo, mDescriptorSets.data()) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for(size_t i = 0; i < count; i++)
    {
      VkDescriptorBufferInfo bufferInfo = {};
      bufferInfo.buffer = mUniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);

      VkWriteDescriptorSet descriptorWrite = {};
      descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet = mDescriptorSets[i];
      descriptorWrite.dstBinding = 0;
      descriptorWrite.dstArrayElement = 0;
      descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrite.descriptorCount = 1;
      descriptorWrite.pBufferInfo = &bufferInfo;
      descriptorWrite.pImageInfo = nullptr; // Optional
      descriptorWrite.pTexelBufferView = nullptr; // Optional

      vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
    }
  }

  void createCommandBuffers()
  {
    CommandBuffersResultData resultData;
    CommandBuffersCreationData creationData;
    creationData.mDevice = mDevice;
    creationData.mCommandPool = mCommandPool;
    creationData.mRenderPass = mRenderPass;
    creationData.mGraphicsPipeline = mGraphicsPipeline;
    creationData.mVertexBuffer = mVertexBuffer;
    creationData.mIndexBuffer = mIndexBuffer;
    creationData.mSwapChain = mSwapChain;
    creationData.mSwapChainExtent = mSwapChainExtent;
    creationData.mIndexBufferCount = static_cast<uint32_t>(indices.size());
    creationData.mSwapChainFramebuffers = mSwapChainFramebuffers;
    creationData.mPipelineLayout = mPipelineLayout;
    creationData.mDescriptorSets = mDescriptorSets;

    CreateCommandBuffers(creationData, resultData);

    mCommandBuffers = resultData.mCommandBuffers;
    mCommandBuffers.resize(mSwapChainFramebuffers.size());
  }

  void createSyncObjects()
  {
    mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      if(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(mDevice, &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS)
      {
        throw std::runtime_error("failed to create semaphores!");
      }
    }

  }

  void mainLoop() {
    while(!glfwWindowShouldClose(mWindow))
    {
      glfwPollEvents();
      drawFrame();
    }

    vkDeviceWaitIdle(mDevice);
  }

  void drawFrame()
  {
    vkWaitForFences(mDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized)
    {
      mFramebufferResized = false;
      recreateSwapChain();
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      throw std::runtime_error("failed to acquire swap chain image!");

    updateUniformBuffer(imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores[mCurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[mCurrentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(mDevice, 1, &mInFlightFences[mCurrentFrame]);

    if(vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
      throw std::runtime_error("failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {mSwapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    result = vkQueuePresentKHR(mPresentQueue, &presentInfo);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
      recreateSwapChain();
    else if(result != VK_SUCCESS)
      throw std::runtime_error("failed to present swap chain image!");

    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  }

  void updateUniformBuffer(uint32_t currentImage)
  {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo = {};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), mSwapChainExtent.width / (float)mSwapChainExtent.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    void* data;
    vkMapMemory(mDevice, mUniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(mDevice, mUniformBuffersMemory[currentImage]);
  }

  void cleanup() 
  {
    cleanupSwapChain();

    vkDestroyImage(mDevice, mTextureImage, nullptr);
    vkFreeMemory(mDevice, mTextureImageMemory, nullptr);
    vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
    vkDestroyBuffer(mDevice, mIndexBuffer, nullptr);
    vkFreeMemory(mDevice, mIndexBufferMemory, nullptr);
    vkDestroyBuffer(mDevice, mVertexBuffer, nullptr);
    vkFreeMemory(mDevice, mVertexBufferMemory, nullptr);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      vkDestroyFence(mDevice, mInFlightFences[i], nullptr);
      vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
    }

    if(enableValidationLayers)
      DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);

    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);

    vkDestroyDevice(mDevice, nullptr);
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    vkDestroyInstance(mInstance, nullptr);
    glfwDestroyWindow(mWindow);
    glfwTerminate();
  }

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
  VkDescriptorSetLayout mDescriptorSetLayout;
  VkPipelineLayout mPipelineLayout;
  VkPipeline mGraphicsPipeline;
  VkCommandPool mCommandPool;
  VkDescriptorPool mDescriptorPool;
  std::vector<VkDescriptorSet> mDescriptorSets;

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

  std::vector<VkBuffer> mUniformBuffers;
  std::vector<VkDeviceMemory> mUniformBuffersMemory;

  VkImage mTextureImage;
  VkDeviceMemory mTextureImageMemory;
};

int main()
{
  HelloTriangleApplication app;

  try
  {
    app.run();
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}