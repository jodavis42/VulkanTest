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
#include "VulkanInitialization.hpp"
#include "VulkanImages.hpp"
#include "VulkanSwapChain.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "VulkanStructures.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

const int cWidth = 800;
const int cHeight = 600;
const std::string MODEL_PATH = "models/chalet.obj";
const std::string TEXTURE_PATH = "textures/chalet.jpg";
const int MAX_FRAMES_IN_FLIGHT = 2;
const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct PerCameraData {
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};

struct PerObjectData {
  alignas(16) glm::mat4 model;
};

struct Model
{
  String mMaterialName;
  String mMeshName;

  Vec3 mTranslation = Vec3(0, 0, 0);
  glm::mat3 mRotation;
  Vec3 mScale = Vec3(1, 1, 1);
};

struct VulkanUniformBuffer
{
  VkBuffer mBuffer;
  VkDeviceMemory mBufferMemory;
};

struct VulkanUniformBuffers
{
  std::vector<VulkanUniformBuffer> mBuffers;
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

  void GlobalSetup()
  {
    CreateInstance(mInstance);
    SetupDebugMessenger(mInstance, mDebugMessenger);
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPool();
    createSyncObjects();
  }

  void populateMaterialBuffer()
  {
    void* data = nullptr;
    vkMapMemory(mDevice, mMaterialBuffer.mBufferMemory, 0, mDeviceLimits.mMaxUniformBufferRange, 0, &data);
    unsigned char* byteData = static_cast<unsigned char*>(data);

    float value = 0.5f;
    glm::vec4 color(value);
    VulkanMaterial* vulkanMaterial = mVulkanMaterialMap["Test"];
    memcpy(byteData + vulkanMaterial->mBufferOffset, &color, vulkanMaterial->mBufferSize);

    vkUnmapMemory(mDevice, mMaterialBuffer.mBufferMemory);
  }

  void LoadModelsAndBuffersAndTextures()
  {
    mMeshManager.Load();
    loadModel();
    
    createTextureSampler();
    createTextureImage();
    createTextureImageView();

    populateMaterialBuffer();
  }

  void createMaterialBuffers()
  {
    VkDeviceSize materialBufferSize = mDeviceLimits.mMaxUniformBufferRange;
    VulkanBufferCreationData vulkanData{mPhysicalDevice, mDevice, mGraphicsQueue, mGraphicsPipeline, mCommandPool};
    VkBufferUsageFlags usageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    CreateBuffer(vulkanData, materialBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, usageFlags, mMaterialBuffer.mBuffer, mMaterialBuffer.mBufferMemory);
  }

  void initVulkan()
  {
    GlobalSetup();

    createSwapChainImageAndViews();
    createDepthResources();
    createMaterialBuffers();

    LoadModelsAndBuffersAndTextures();
    

    createRenderPass();

    // Create shader
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();

    createFramebuffers();
    createGraphicsPipeline();
    
    createCommandBuffers();
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

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
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
    QueryPhysicalDeviceLimits(mPhysicalDevice, mDeviceLimits);
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

  void CleanupUniform(VulkanUniformBuffer& buffer)
  {
    vkDestroyBuffer(mDevice, buffer.mBuffer, nullptr);
    vkFreeMemory(mDevice, buffer.mBufferMemory, nullptr);
  }

  void CleanupUniforms(VulkanUniformBuffers& buffers)
  {
    for(auto&& buffer : buffers.mBuffers)
      CleanupUniform(buffer);
  }

  void cleanupSwapChain()
  {
    for(auto framebuffer : mSwapChainFramebuffers)
      vkDestroyFramebuffer(mDevice, framebuffer, nullptr);

    vkFreeCommandBuffers(mDevice, mCommandPool, static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());

    vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
    vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

    for(auto imageView : mSwapChain.mImageViews)
      vkDestroyImageView(mDevice, imageView, nullptr);

    vkDestroySwapchainKHR(mDevice, mSwapChain.mSwapChain, nullptr);

    CleanupUniforms(mUniformBuffers);
    CleanupUniform(mMaterialBuffer);

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
    createSwapChainImageAndViews();

    createRenderPass();
    createGraphicsPipeline();
    createDepthResources();
    createFramebuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
  }

  static void SwapChainQuerySizeCallback(uint32_t& width, uint32_t& height, void* userData)
  {
    HelloTriangleApplication* self = static_cast<HelloTriangleApplication*>(userData);

    int w, h;
    glfwGetFramebufferSize(self->mWindow, &w, &h);
    width = static_cast<uint32_t>(w);
    height = static_cast<uint32_t>(h);
  }

  void createSwapChainImageAndViews()
  {
    SwapChainResultInfo swapChainResultInfo;

    SwapChainCreationInfo swapChainInfo;
    swapChainInfo.mDevice = mDevice;
    swapChainInfo.mPhysicalDevice = mPhysicalDevice;
    swapChainInfo.mSurface = mSurface;
    swapChainInfo.mUserData = this;
    swapChainInfo.mQueryFn = &HelloTriangleApplication::SwapChainQuerySizeCallback;

    CreateSwapChainAndViews(swapChainInfo, mSwapChain);
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
    creationData.mSwapChainImageFormat = mSwapChain.mImageFormat;
    creationData.mDepthFormat = findDepthFormat();
    CreateRenderPass(creationData);

    mRenderPass = creationData.mRenderPass;
  }

  void createGraphicsPipeline()
  {
    VulkanMaterial* vMaterial = mVulkanMaterialMap["Test"];
    auto vertexShaderCode = readFile("shaders/vertex.spv");
    auto pixelShaderCode = readFile("shaders/pixel.spv");

    GraphicsPipelineData graphicsPipelineData;
    graphicsPipelineData.mDevice = mDevice;
    graphicsPipelineData.mGraphicsPipeline = mGraphicsPipeline;
    graphicsPipelineData.mPipelineLayout = mPipelineLayout;
    graphicsPipelineData.mPixelShaderCode = pixelShaderCode;
    graphicsPipelineData.mVertexShaderCode = vertexShaderCode;
    graphicsPipelineData.mRenderPass = mRenderPass;
    graphicsPipelineData.mSwapChainExtent = mSwapChain.mExtent;
    graphicsPipelineData.mVertexAttributeDescriptions = VulkanVertex::getAttributeDescriptions();
    graphicsPipelineData.mVertexBindingDescriptions = VulkanVertex::getBindingDescription();
    graphicsPipelineData.mDescriptorSetLayout = vMaterial->mDescriptorSetLayout;
    CreateGraphicsPipeline(graphicsPipelineData);

    mGraphicsPipeline = graphicsPipelineData.mGraphicsPipeline;
    mPipelineLayout = graphicsPipelineData.mPipelineLayout;
  }

  void createFramebuffers()
  {
    mSwapChainFramebuffers.resize(GetFrameCount());

    for(size_t i = 0; i < GetFrameCount(); i++)
    {
      std::array<VkImageView, 2> attachments = {mSwapChain.mImageViews[i], mDepthSet.mImageView};

      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = mRenderPass;
      framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = mSwapChain.mExtent.width;
      framebufferInfo.height = mSwapChain.mExtent.height;
      framebufferInfo.layers = 1;

      if(vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS)
        throw std::runtime_error("failed to create framebuffer!");
    }
  }

  void createCommandPool()
  {
    CreateCommandPool(mPhysicalDevice, mDevice, mSurface, mCommandPool);
  }

  void createDepthResources()
  {
    VkFormat depthFormat = findDepthFormat();
    createImage(mSwapChain.mExtent.width, mSwapChain.mExtent.height, mMipLevels, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthSet.mImage, mDepthSet.mImageMemory);

    ImageViewCreationInfo viewCreationInfo(mDevice, mDepthSet.mImage);
    viewCreationInfo.mFormat = depthFormat;
    viewCreationInfo.mViewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreationInfo.mAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewCreationInfo.mMipLevels = mMipLevels;
    VulkanStatus status = CreateImageView(viewCreationInfo, mDepthSet.mImageView);

    ImageLayoutTransitionInfo transitionInfo;
    transitionInfo.mDevice = mDevice;
    transitionInfo.mGraphicsQueue = mGraphicsQueue;
    transitionInfo.mCommandPool = mCommandPool;
    transitionInfo.mFormat = depthFormat;
    transitionInfo.mImage = mDepthSet.mImage;
    transitionInfo.mOldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    transitionInfo.mNewLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    transitionInfo.mMipLevels = mMipLevels;
    TransitionImageLayout(transitionInfo);
  }

  VkFormat findDepthFormat()
  {
    return findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
  }

  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
  {
    for(VkFormat format : candidates)
    {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);

      if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        return format;
      else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        return format;
    }

    throw std::runtime_error("failed to find supported format!");
  }

  void createTextureImage()
  {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    uint32_t pixelsSize = texWidth * texHeight * 4;
    uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    TextureImageCreationInfo info;
    info.mPhysicalDevice = mPhysicalDevice;
    info.mDevice = mDevice;
    info.mGraphicsQueue = mGraphicsQueue;
    info.mGraphicsPipeline = mGraphicsPipeline;
    info.mCommandPool = mCommandPool;
    info.mFormat = VK_FORMAT_R8G8B8A8_SRGB;
    info.mPixels = (void*)pixels;
    info.mPixelsSize = pixelsSize;
    info.mWidth = texWidth;
    info.mHeight = texHeight;
    info.mMipLevels = mipLevels;
    CreateTextureImage(info, mTextureSet);

    stbi_image_free(pixels);
  }

  void createTextureImageView()
  {
    ImageViewCreationInfo info(mDevice, mTextureSet.mImage);
    info.mFormat = VK_FORMAT_R8G8B8A8_SRGB;
    info.mViewType = VK_IMAGE_VIEW_TYPE_2D;
    info.mAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    info.mMipLevels = mMipLevels;
    VulkanStatus status = CreateImageView(info, mTextureSet.mImageView);
  }

  void createTextureSampler()
  {
    SamplerCreationInfo info;
    info.mDevice = mDevice;
    info.mMaxLod = static_cast<float>(mMipLevels);
    CreateTextureSampler(info, mSampler);
  }

  void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
  {
    ImageCreationInfo imageInfo;
    imageInfo.mDevice = mDevice;
    imageInfo.mWidth = width;
    imageInfo.mHeight = height;
    imageInfo.mMipLevels = mipLevels;
    imageInfo.mFormat = format;
    imageInfo.mTiling = tiling;
    imageInfo.mUsage = usage;
    imageInfo.mType = VK_IMAGE_TYPE_2D;
    CreateImage(imageInfo, image);

    ImageMemoryCreationInfo memoryInfo;
    memoryInfo.mImage = image;
    memoryInfo.mDevice = mDevice;
    memoryInfo.mPhysicalDevice = mPhysicalDevice;
    memoryInfo.mProperties = properties;
    CreateImageMemory(memoryInfo, imageMemory);

    vkBindImageMemory(mDevice, image, imageMemory, 0);
  }

  void CreateVertexBuffer(Mesh* mesh, VulkanMesh* vulkanMesh)
  {
    VulkanBufferCreationData vulkanData{mPhysicalDevice, mDevice, mGraphicsQueue, mGraphicsPipeline, mCommandPool};
    VkDeviceSize bufferSize = sizeof(mesh->mVertices[0]) * mesh->mVertices.size();
    CreateBuffer(vulkanData, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vulkanMesh->mVertexBuffer, vulkanMesh->mVertexBufferMemory, mesh->mVertices.data(), bufferSize);
  }

  void CreateIndexBuffer(Mesh* mesh, VulkanMesh* vulkanMesh)
  {
    VulkanBufferCreationData vulkanData{mPhysicalDevice, mDevice, mGraphicsQueue, mGraphicsPipeline, mCommandPool};
    vulkanMesh->mIndexCount = static_cast<uint32_t>(mesh->mIndices.size());
    VkDeviceSize bufferSize = sizeof(mesh->mIndices[0]) * mesh->mIndices.size();
    CreateBuffer(vulkanData, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vulkanMesh->mIndexBuffer, vulkanMesh->mIndexBufferMemory, mesh->mIndices.data(), bufferSize);
  }

  void LoadVulkanMesh(const String& name, Mesh* mesh)
  {
    VulkanMesh* vulkanMesh = new VulkanMesh();
    CreateVertexBuffer(mesh, vulkanMesh);
    CreateIndexBuffer(mesh, vulkanMesh);
    mVulkanMeshMap[name] = vulkanMesh;
  }

  VulkanStatus LoadVulkanMaterialDescriptorSetLayoutBinding(VulkanMaterial* vulkanMaterial)
  {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding cameraDataLayoutBinding = {};
    cameraDataLayoutBinding.binding = 1;
    cameraDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    cameraDataLayoutBinding.descriptorCount = 1;
    cameraDataLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    cameraDataLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding materialLayoutBinding = {};
    materialLayoutBinding.binding = 2;
    materialLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    materialLayoutBinding.descriptorCount = 1;
    materialLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    materialLayoutBinding.pImmutableSamplers = nullptr; // Optional
    vulkanMaterial->mBufferOffset = 0;
    vulkanMaterial->mBufferSize = sizeof(glm::vec4);

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 3;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {uboLayoutBinding, cameraDataLayoutBinding, materialLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    VulkanStatus result;
    if(vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &vulkanMaterial->mDescriptorSetLayout) != VK_SUCCESS)
      result.MarkFailed("failed to create descriptor set layout!");
    return result;
  }

  void LoadVulkanMaterial(Material* material, VulkanMaterial* vulkanMaterial)
  {
    auto vertexShaderCode = readFile(material->mVertexShaderName);
    auto pixelShaderCode = readFile(material->mPixelShaderName);

    vulkanMaterial->mVertexShaderModule = CreateShaderModule(mDevice, vertexShaderCode);
    vulkanMaterial->mPixelShaderModule = CreateShaderModule(mDevice, pixelShaderCode);
    LoadVulkanMaterialDescriptorSetLayoutBinding(vulkanMaterial);
  }

  void LoadMaterial(const String& name, Material*& material, VulkanMaterial*& vulkanMaterial)
  {
    material = new Material();
    material->mVertexShaderName = "shaders/vertex.spv";
    material->mPixelShaderName = "shaders/pixel.spv";
    mMaterialMap[name] = material;

    vulkanMaterial = new VulkanMaterial();
    LoadVulkanMaterial(material, vulkanMaterial);
    mVulkanMaterialMap[name] = vulkanMaterial;
  }

  void LoadModel(const String& name, const String& path)
  {
    Mesh* mesh = mMeshManager.mMeshMap[name];
    LoadVulkanMesh(name, mesh);

    Material* material = nullptr;
    VulkanMaterial* vulkanMaterial = nullptr;
    LoadMaterial(name, material, vulkanMaterial);

    Model* model = new Model();
    model->mMeshName = name;
    model->mMaterialName = name;
    model->mTranslation = Vec3(0, 0, 0);
    mModels.emplace_back(model);

    model = new Model();
    model->mMeshName = name;
    model->mMaterialName = name;
    model->mTranslation = Vec3(3, 0, 0);
    mModels.emplace_back(model);

    model = new Model();
    model->mMeshName = name;
    model->mMaterialName = name;
    model->mTranslation = Vec3(-3, 0, 0);
    mModels.emplace_back(model);
  }

  void loadModel()
  {
    LoadModel("Test", MODEL_PATH);
  }

  void createUniformBuffers()
  {
    VkDeviceSize bufferSize = AlignUniformBufferOffset(sizeof(PerCameraData)) + sizeof(PerObjectData) * 1000;

    size_t count = GetFrameCount();
    mUniformBuffers.mBuffers.resize(count);

    VulkanBufferCreationData vulkanData{mPhysicalDevice, mDevice, mGraphicsQueue, mGraphicsPipeline, mCommandPool};

    for(size_t i = 0; i < count; i++)
    {
      VulkanUniformBuffer& buffer = mUniformBuffers.mBuffers[i];
      VkImageUsageFlags usageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      CreateBuffer(vulkanData, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, usageFlags, buffer.mBuffer, buffer.mBufferMemory);
    }
  }

  void createDescriptorPool()
  {
    uint32_t swapChainCount = static_cast<uint32_t>(GetFrameCount());
    std::array<VkDescriptorPoolSize, 3> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = swapChainCount;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = swapChainCount;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[2].descriptorCount = swapChainCount * 2;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = swapChainCount;

    if(vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
      throw std::runtime_error("failed to create descriptor pool!");
  }

  size_t AlignUniformBufferOffset(size_t offset)
  {
    size_t alignment = mDeviceLimits.mMinUniformBufferOffsetAlignment;
    size_t extra = offset % alignment;
    size_t result = offset / alignment + extra != 0 ? alignment : 0;
    return result;
  }

  void createDescriptorSets()
  {
    VulkanMaterial* vMaterial = mVulkanMaterialMap["Test"];
    uint32_t count = static_cast<uint32_t>(GetFrameCount());
    std::vector<VkDescriptorSetLayout> layouts(count, vMaterial->mDescriptorSetLayout);
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
      VulkanUniformBuffer& buffer = mUniformBuffers.mBuffers[i];
      VkDescriptorBufferInfo perCameraInfo = {};
      perCameraInfo.buffer = buffer.mBuffer;
      perCameraInfo.offset = 0;
      perCameraInfo.range = sizeof(PerCameraData);

      VkDescriptorBufferInfo perObjectInfo = {};
      perObjectInfo.buffer = buffer.mBuffer;
      perObjectInfo.offset = AlignUniformBufferOffset(sizeof(PerCameraData));
      perObjectInfo.range = sizeof(PerObjectData);

      VkDescriptorBufferInfo materialInfo = {};
      materialInfo.buffer = mMaterialBuffer.mBuffer;
      materialInfo.offset = 0;
      materialInfo.range = vMaterial->mBufferSize;

      VkDescriptorImageInfo imageInfo = {};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = mTextureSet.mImageView;
      imageInfo.sampler = mSampler;

      std::array<VkWriteDescriptorSet, 4> descriptorWrites = {};

      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = mDescriptorSets[i];
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &perCameraInfo;

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = mDescriptorSets[i];
      descriptorWrites[1].dstBinding = 1;
      descriptorWrites[1].dstArrayElement = 0;
      descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
      descriptorWrites[1].descriptorCount = 1;
      descriptorWrites[1].pBufferInfo = &perObjectInfo;

      descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[2].dstSet = mDescriptorSets[i];
      descriptorWrites[2].dstBinding = 2;
      descriptorWrites[2].dstArrayElement = 0;
      descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
      descriptorWrites[2].descriptorCount = 1;
      descriptorWrites[2].pBufferInfo = &materialInfo;

      descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[3].dstSet = mDescriptorSets[i];
      descriptorWrites[3].dstBinding = 3;
      descriptorWrites[3].dstArrayElement = 0;
      descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptorWrites[3].descriptorCount = 1;
      descriptorWrites[3].pImageInfo = &imageInfo;

      uint32_t descriptorsCount = static_cast<uint32_t>(descriptorWrites.size());
      vkUpdateDescriptorSets(mDevice, descriptorsCount, descriptorWrites.data(), 0, nullptr);
    }
  }

  void createCommandBuffers()
  {
    VulkanMesh* vMesh = mVulkanMeshMap["Test"];
    CommandBuffersResultData resultData;
    CommandBuffersCreationData creationData;
    creationData.mDevice = mDevice;
    creationData.mCommandPool = mCommandPool;
    creationData.mRenderPass = mRenderPass;
    creationData.mGraphicsPipeline = mGraphicsPipeline;
    creationData.mVertexBuffer = vMesh->mVertexBuffer;
    creationData.mIndexBuffer = vMesh->mIndexBuffer;
    creationData.mSwapChain = mSwapChain.mSwapChain;
    creationData.mSwapChainExtent = mSwapChain.mExtent;
    creationData.mIndexBufferCount = vMesh->mIndexCount;
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

  uint32_t GetFrameCount()
  {
    return mSwapChain.GetCount();
  }

  struct FrameData
  {
    VkDescriptorSet mDescriptorSet;
    VkFramebuffer mFramebuffer;
    VkCommandBuffer mCommandBuffer;
    VkBuffer mUniformBuffer;
    VkDeviceMemory mUniformBufferMemory;
  };

  void prepareFrame(FrameData& frameData)
  {
    updateUniformBuffer(frameData.mUniformBufferMemory);

    VulkanMesh* mesh = mVulkanMeshMap["Test"];
    VulkanMaterial* material = mVulkanMaterialMap["Test"];

    VkCommandBuffer commandBuffer = frameData.mCommandBuffer;

    uint32_t dynamicOffsets[2] = 
    {
      static_cast<uint32_t>(AlignUniformBufferOffset(sizeof(PerObjectData))),
      0
    };

    uint32_t dynamicOffsetBase[2] = {0, 0};
    CommandBufferWriteInfo writeInfo;
    writeInfo.mDevice = mDevice;
    writeInfo.mCommandPool = mCommandPool;
    writeInfo.mRenderPass = mRenderPass;
    writeInfo.mGraphicsPipeline = mGraphicsPipeline;
    writeInfo.mPipelineLayout = mPipelineLayout;
    writeInfo.mVertexBuffer = mesh->mVertexBuffer;
    writeInfo.mIndexBuffer = mesh->mIndexBuffer;
    writeInfo.mSwapChain = mSwapChain.mSwapChain;
    writeInfo.mSwapChainExtent = mSwapChain.mExtent;
    writeInfo.mIndexBufferCount = mesh->mIndexCount;
    writeInfo.mSwapChainFramebuffer = frameData.mFramebuffer;
    writeInfo.mDescriptorSet = frameData.mDescriptorSet;
    writeInfo.mDrawCount = static_cast<uint32_t>(mModels.size());
    writeInfo.mDynamicOffsetsCount = 2;
    writeInfo.mDynamicOffsetsBase = dynamicOffsetBase;
    writeInfo.mDynamicOffsets = dynamicOffsets;
    WriteCommandBuffer(writeInfo, commandBuffer);
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
    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain.mSwapChain, UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mFramebufferResized)
    {
      mFramebufferResized = false;
      recreateSwapChain();
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
      throw std::runtime_error("failed to acquire swap chain image!");

    FrameData frameData;
    frameData.mCommandBuffer = mCommandBuffers[imageIndex];
    frameData.mDescriptorSet = mDescriptorSets[imageIndex];
    frameData.mFramebuffer = mSwapChainFramebuffers[imageIndex];
    frameData.mUniformBuffer = mUniformBuffers.mBuffers[imageIndex].mBuffer;
    frameData.mUniformBufferMemory = mUniformBuffers.mBuffers[imageIndex].mBufferMemory;
    prepareFrame(frameData);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores[mCurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frameData.mCommandBuffer;

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

    VkSwapchainKHR swapChains[] = {mSwapChain.mSwapChain};
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

  void updateUniformBuffer(VkDeviceMemory uniformBufferMemory)
  {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    PerCameraData perCameraData;
    perCameraData.view = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    perCameraData.proj = glm::perspective(glm::radians(45.0f), mSwapChain.mExtent.width / (float)mSwapChain.mExtent.height, 0.1f, 10.0f);
    perCameraData.proj[1][1] *= -1;

    void* data;

    size_t offset = 0;
    vkMapMemory(mDevice, uniformBufferMemory, offset, sizeof(perCameraData), 0, &data);
    memcpy(data, &perCameraData, sizeof(perCameraData));
    vkUnmapMemory(mDevice, uniformBufferMemory);
    offset += AlignUniformBufferOffset(sizeof(perCameraData));

    size_t count = mModels.size();
    vkMapMemory(mDevice, uniformBufferMemory, offset, AlignUniformBufferOffset(sizeof(PerObjectData)) * count, 0, &data);
    for(size_t i = 0; i < count; ++i)
    {
      Model* model = mModels[i];
      glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(model->mScale.x, model->mScale.y, model->mScale.z));
      glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
      glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(model->mTranslation.x, model->mTranslation.y, model->mTranslation.z));
      glm::mat4 transform = translation * rotation * scale;
      char* memory = ((char*)data) + AlignUniformBufferOffset(sizeof(PerObjectData)) * i;
      memcpy(memory, &transform, sizeof(transform));
    }
    vkUnmapMemory(mDevice, uniformBufferMemory);
  }

  void updateUniformBuffer(uint32_t currentImage)
  {
    updateUniformBuffer(mUniformBuffers.mBuffers[currentImage].mBufferMemory);
  }

  void cleanup() 
  {
    cleanupSwapChain();

    vkDestroySampler(mDevice, mSampler, nullptr);
    Cleanup(mDevice, mDepthSet);
    Cleanup(mDevice, mTextureSet);
    for(auto pair : mVulkanMaterialMap)
    {
      VulkanMaterial* vMaterial = pair.second;
      vkDestroyDescriptorSetLayout(mDevice, vMaterial->mDescriptorSetLayout, nullptr);
      vkDestroyShaderModule(mDevice, vMaterial->mPixelShaderModule, nullptr);
      vkDestroyShaderModule(mDevice, vMaterial->mVertexShaderModule, nullptr);
    }

    for(auto pair : mVulkanMeshMap)
    {
      VulkanMesh* vMesh = pair.second;
      vkDestroyBuffer(mDevice, vMesh->mIndexBuffer, nullptr);
      vkFreeMemory(mDevice, vMesh->mIndexBufferMemory, nullptr);
      vkDestroyBuffer(mDevice, vMesh->mVertexBuffer, nullptr);
      vkFreeMemory(mDevice, vMesh->mVertexBufferMemory, nullptr);
    }

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
  PhysicalDeviceLimits mDeviceLimits;
  SwapChainData mSwapChain;

  VkRenderPass mRenderPass;
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

  std::unordered_map<String, Mesh*> mMeshMap;
  std::unordered_map<String, VulkanMesh*> mVulkanMeshMap;
  std::unordered_map<String, Material*> mMaterialMap;
  std::unordered_map<String, VulkanMaterial*> mVulkanMaterialMap;
  std::vector<Model*> mModels;

  VulkanUniformBuffers mUniformBuffers;
  VulkanUniformBuffer mMaterialBuffer;

  uint32_t mMipLevels = 1;
  ImageViewMemorySet mTextureSet;
  VkSampler mSampler;
  ImageViewMemorySet mDepthSet;
  MeshManager mMeshManager;
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