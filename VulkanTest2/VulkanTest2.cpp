#include "pch.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
#include "Helpers/File.hpp"
#include "VulkanExtensions.hpp"
#include "VulkanBufferCreation.hpp"
#include "VulkanValidationLayers.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanRenderPass.hpp"
#include "Helpers/Vertex.hpp"
#include "VulkanCommandBuffer.hpp"
#include "VulkanPhysicsDeviceSelection.hpp"
#include "VulkanDeviceQueries.hpp"
#include "VulkanLogicalDeviceCreation.hpp"
#include "VulkanInitialization.hpp"
#include "VulkanImages.hpp"
#include "VulkanSwapChain.hpp"
#include "Helpers/Mesh.hpp"
#include "Helpers/Material.hpp"
#include "Helpers/Texture.hpp"
#include "Helpers/Shader.hpp"
#include "VulkanStructures.hpp"
#include "VulkanRenderer.hpp"

const int cWidth = 800;
const int cHeight = 600;
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

  static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
  {
    auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->mFramebufferResized = true;
  }

  static VulkanStatus SurfaceCreationCallback(VkInstance instance, void* userData, VkSurfaceKHR& outSurface)
  {
    auto self = reinterpret_cast<HelloTriangleApplication*>(userData);
    auto result = glfwCreateWindowSurface(instance, self->mWindow, nullptr, &outSurface);

    VulkanStatus status;
    if(result != VK_SUCCESS)
      status.MarkFailed("failed to create window surface!");
    return status;
  }

  static void SwapChainQuerySizeCallback(uint32_t& width, uint32_t& height, void* userData)
  {
    HelloTriangleApplication* self = static_cast<HelloTriangleApplication*>(userData);

    int w, h;
    glfwGetFramebufferSize(self->mWindow, &w, &h);
    width = static_cast<uint32_t>(w);
    height = static_cast<uint32_t>(h);
  }

  void initWindow()
  {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(cWidth, cHeight, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, FramebufferResizeCallback);
  }

  void GlobalSetup()
  {
    int width = 0, height = 0;
    glfwGetFramebufferSize(mWindow, &width, &height);
    auto internal = mRenderer.GetRuntimeData();

    VulkanInitializationData initData;
    initData.mWidth = width;
    initData.mHeight = height;
    initData.mSurfaceCreationCallback.mCallbackFn = &HelloTriangleApplication::SurfaceCreationCallback;
    initData.mSurfaceCreationCallback.mUserData = this;
    mRenderer.Initialize(initData);

    mInstance = internal->mInstance;
    mDebugMessenger = internal->mDebugMessenger;
    mPhysicalDevice = internal->mPhysicalDevice;
    mDevice = internal->mDevice;
    mCommandPool = internal->mCommandPool;
    mGraphicsQueue = internal->mGraphicsQueue;
    mPresentQueue = internal->mPresentQueue;
    mSurface = internal->mSurface;
    mDeviceLimits = internal->mDeviceLimits;
    mSyncObjects = internal->mSyncObjects;
  }

  void populateMaterialBuffer()
  {
    void* data = nullptr;
    vkMapMemory(mDevice, mMaterialBuffer.mBufferMemory, 0, mDeviceLimits.mMaxUniformBufferRange, 0, &data);
    unsigned char* byteData = static_cast<unsigned char*>(data);

    float value = 0.5f;
    glm::vec4 color(value);
    VulkanMaterial* vulkanMaterial = mRenderer.mMaterialMap[mMaterialManager.Find("Test")];
    memcpy(byteData + vulkanMaterial->mBufferOffset, &color, vulkanMaterial->mBufferSize);

    vkUnmapMemory(mDevice, mMaterialBuffer.mBufferMemory);
  }

  void LoadVulkanImage(const String& name, Texture* texture)
  {
    mRenderer.CreateTexture(texture);
  }

  void LoadVulkanShader(const String& name, Shader* shader)
  {
    mRenderer.CreateShader(shader);
  }

  void LoadVulkanShaders()
  {
    for(auto pair : mShaderManager.mShaderMap)
    {
      mRenderer.CreateShader(pair.second);
    }
  }

  void LoadVulkanMaterial(Material* material)
  {
    mRenderer.CreateMaterial(material, mGlobalDescriptorLayouts.data(), mGlobalDescriptorLayouts.size());
  }

  void LoadVulkanMaterials()
  {
    for(auto pair : mMaterialManager.mMaterialMap)
    {
      LoadVulkanMaterial(pair.second);
    }
  }

  void CreateGlobalMaterialDescriptorSets()
  {
    mGlobalDescriptorLayouts.resize(2);
    MaterialDescriptorSetLayout& cameraDescriptor = mGlobalDescriptorLayouts[0];
    cameraDescriptor.mBinding = 0;
    cameraDescriptor.mDescriptorType = MaterialDescriptorType::Uniform;
    cameraDescriptor.mStageFlags = (ShaderStageFlags::Enum)(ShaderStageFlags::Vertex);
    size_t offset = 0;
    offset = cameraDescriptor.AddElement(MaterialDescriptorEntryType::Mat4, offset, sizeof(glm::mat4));
    offset = cameraDescriptor.AddElement(MaterialDescriptorEntryType::Mat4, offset, sizeof(glm::mat4));

    MaterialDescriptorSetLayout& transformDescriptor = mGlobalDescriptorLayouts[1];
    transformDescriptor.mBinding = 1;
    transformDescriptor.mDescriptorType = MaterialDescriptorType::UniformDynamic;
    transformDescriptor.mStageFlags = (ShaderStageFlags::Enum)(ShaderStageFlags::Vertex);
    offset = 0;
    offset = transformDescriptor.AddElement(MaterialDescriptorEntryType::Mat4, offset, sizeof(glm::mat4));
  }

  void LoadShadersAndMaterials()
  {
    CreateGlobalMaterialDescriptorSets();

    ShaderLoadData shaderLoadData;
    shaderLoadData.mShaderCodePaths[ShaderStage::Vertex] = "shaders/vertex.spv";
    shaderLoadData.mShaderCodePaths[ShaderStage::Pixel] = "shaders/pixel.spv";
    mShaderManager.LoadShader("Test", shaderLoadData);

    Material* material = new Material();
    
    material->mDescriptorLayouts.resize(2);
    MaterialDescriptorSetLayout& materialDescriptor = material->mDescriptorLayouts[0];
    materialDescriptor.mBinding = 2;
    materialDescriptor.mStageFlags = (ShaderStageFlags::Enum)(ShaderStageFlags::Vertex | ShaderStageFlags::Pixel);
    materialDescriptor.mDescriptorType = MaterialDescriptorType::UniformDynamic;
    materialDescriptor.AddElement(MaterialDescriptorEntryType::Vec4, 0, sizeof(glm::vec4));

    material->mBuffers.resize(1);
    MaterialBuffer& materialBuffer = material->mBuffers[0];
    materialBuffer.mBinding = 2;
    materialBuffer.mData.resize(sizeof(glm::vec4));
    glm::vec4* color = (glm::vec4*) materialBuffer.mData.data();
    *color = glm::vec4(0.5f);

    MaterialDescriptorSetLayout& imageDescriptor = material->mDescriptorLayouts[1];
    imageDescriptor.mBinding = 3;
    imageDescriptor.mStageFlags = (ShaderStageFlags::Enum)(ShaderStageFlags::Pixel);
    imageDescriptor.mDescriptorType = MaterialDescriptorType::SampledImage;
    
    material->mShader = mShaderManager.mShaderMap["Test"];
    mMaterialManager.Add("Test", material);
  }

  void LoadModelsAndBuffersAndTextures()
  {
    mMeshManager.Load();
    mTextureManager.Load();
    LoadShadersAndMaterials();

    loadModel();
    LoadVulkanImage("Test", mTextureManager.Find("Test"));
    LoadVulkanShaders();
    LoadVulkanMaterials();

    populateMaterialBuffer();
  }

  void createMaterialBuffers()
  {
    VkDeviceSize materialBufferSize = mDeviceLimits.mMaxUniformBufferRange;
    VulkanBufferCreationData vulkanData{mPhysicalDevice, mDevice, mGraphicsQueue, mCommandPool};
    VkBufferUsageFlags usageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    CreateBuffer(vulkanData, materialBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, usageFlags, mMaterialBuffer.mBuffer, mMaterialBuffer.mBufferMemory);
  }

  void initVulkan()
  {
    GlobalSetup();

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

    mRenderer.DestroySwapChainInternal();

    CleanupUniforms(mUniformBuffers);

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
    mRenderer.Resize(width, height);
    mRenderer.CreateSwapChainInternal();

    createRenderPass();
    createGraphicsPipeline();
    
    createFramebuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
  }

  void createRenderPass()
  {
    RenderPassCreationData creationData;
    creationData.mRenderPass = mRenderPass;
    creationData.mDevice = mDevice;
    creationData.mSwapChainImageFormat = mRenderer.mInternal->mSwapChain.mImageFormat;
    creationData.mDepthFormat = FindDepthFormat(mPhysicalDevice);
    CreateRenderPass(creationData);

    mRenderPass = creationData.mRenderPass;
  }

  void createGraphicsPipeline()
  {
    VulkanMaterial* vMaterial = mRenderer.mMaterialMap[mMaterialManager.Find("Test")];

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    CreatePipelineLayout(mDevice, &vMaterial->mDescriptorSetLayout, 1, mPipelineLayout);

    GraphicsPipelineCreationInfo creationInfo;
    creationInfo.mVertexShaderModule = vMaterial->mVertexShaderModule;
    creationInfo.mPixelShaderModule = vMaterial->mPixelShaderModule;
    creationInfo.mVertexShaderMainFnName = "main";
    creationInfo.mPixelShaderMainFnName = "main";
    creationInfo.mDevice = mDevice;
    creationInfo.mPipelineLayout = mPipelineLayout;
    creationInfo.mRenderPass = mRenderPass;
    creationInfo.mViewportSize = Vec2((float)mRenderer.mInternal->mSwapChain.mExtent.width, (float)mRenderer.mInternal->mSwapChain.mExtent.height);
    creationInfo.mVertexAttributeDescriptions = VulkanVertex::getAttributeDescriptions();
    creationInfo.mVertexBindingDescriptions = VulkanVertex::getBindingDescription();
    CreateGraphicsPipeline(creationInfo, mGraphicsPipeline);
  }

  void createFramebuffers()
  {
    VulkanRuntimeData* runtimeData = mRenderer.GetRuntimeData();
    mSwapChainFramebuffers.resize(GetFrameCount());

    for(size_t i = 0; i < GetFrameCount(); i++)
    {
      std::array<VkImageView, 2> attachments = {mRenderer.mInternal->mSwapChain.mImageViews[i], runtimeData->mDepthImage.mImageView};

      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = mRenderPass;
      framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = mRenderer.mInternal->mSwapChain.mExtent.width;
      framebufferInfo.height = mRenderer.mInternal->mSwapChain.mExtent.height;
      framebufferInfo.layers = 1;

      if(vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapChainFramebuffers[i]) != VK_SUCCESS)
        throw std::runtime_error("failed to create framebuffer!");
    }
  }

  void LoadVulkanMesh(const String& name, Mesh* mesh)
  {
    mRenderer.CreateMesh(mesh);
  }

  void LoadModel(const String& name)
  {
    Mesh* mesh = mMeshManager.mMeshMap[name];
    LoadVulkanMesh(name, mesh);

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
    LoadModel("Test");
  }

  void createUniformBuffers()
  {
    VkDeviceSize bufferSize = AlignUniformBufferOffset(sizeof(PerCameraData)) + sizeof(PerObjectData) * 1000;

    size_t count = GetFrameCount();
    mUniformBuffers.mBuffers.resize(count);

    VulkanBufferCreationData vulkanData{mPhysicalDevice, mDevice, mGraphicsQueue, mCommandPool};

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
    VulkanMaterial* vMaterial = mRenderer.mMaterialMap[mMaterialManager.Find("Test")];
    VulkanImage* vulkanImage = mRenderer.mTextureMap[mTextureManager.Find("Test")];
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
      imageInfo.imageView = vulkanImage->mImageView;
      imageInfo.sampler = vulkanImage->mSampler;

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
    VulkanMesh* vMesh = mRenderer.mMeshMap[mMeshManager.Find("Test")];
    CommandBuffersResultData resultData;
    CommandBuffersCreationData creationData;
    creationData.mDevice = mDevice;
    creationData.mCommandPool = mCommandPool;
    creationData.mRenderPass = mRenderPass;
    creationData.mGraphicsPipeline = mGraphicsPipeline;
    creationData.mVertexBuffer = vMesh->mVertexBuffer;
    creationData.mIndexBuffer = vMesh->mIndexBuffer;
    creationData.mSwapChain = mRenderer.mInternal->mSwapChain.mSwapChain;
    creationData.mSwapChainExtent = mRenderer.mInternal->mSwapChain.mExtent;
    creationData.mIndexBufferCount = vMesh->mIndexCount;
    creationData.mSwapChainFramebuffers = mSwapChainFramebuffers;
    creationData.mPipelineLayout = mPipelineLayout;
    creationData.mDescriptorSets = mDescriptorSets;

    CreateCommandBuffers(creationData, resultData);

    mCommandBuffers = resultData.mCommandBuffers;
    mCommandBuffers.resize(mSwapChainFramebuffers.size());
  }

  uint32_t GetFrameCount()
  {
    return mRenderer.mInternal->mSwapChain.GetCount();
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

    VulkanMesh* mesh = mRenderer.mMeshMap[mMeshManager.Find("Test")];
    VulkanMaterial* material = mRenderer.mMaterialMap[mMaterialManager.Find("Test")];

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
    writeInfo.mSwapChain = mRenderer.mInternal->mSwapChain.mSwapChain;
    writeInfo.mSwapChainExtent = mRenderer.mInternal->mSwapChain.mExtent;
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
    vkWaitForFences(mDevice, 1, &mSyncObjects.mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(mDevice, mRenderer.mInternal->mSwapChain.mSwapChain, UINT64_MAX, mSyncObjects.mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
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

    VkSemaphore waitSemaphores[] = {mSyncObjects.mImageAvailableSemaphores[mCurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frameData.mCommandBuffer;

    VkSemaphore signalSemaphores[] = {mSyncObjects.mRenderFinishedSemaphores[mCurrentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(mDevice, 1, &mSyncObjects.mInFlightFences[mCurrentFrame]);

    if(vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mSyncObjects.mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
      throw std::runtime_error("failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {mRenderer.mInternal->mSwapChain.mSwapChain};
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
    perCameraData.proj = glm::perspective(glm::radians(45.0f), mRenderer.mInternal->mSwapChain.mExtent.width / (float)mRenderer.mInternal->mSwapChain.mExtent.height, 0.1f, 10.0f);
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
    CleanupUniform(mMaterialBuffer);
    mRenderer.Cleanup();

    for(auto pair : mShaderManager.mShaderMap)
    {
      Shader* shader= pair.second;
      mRenderer.DestroyShader(shader);
    }
    for(auto pair : mMaterialManager.mMaterialMap)
    {
      Material* material = pair.second;
      mRenderer.DestroyMaterial(material);
    }
    for(auto pair : mMeshManager.mMeshMap)
    {
      Mesh* mesh = pair.second;
      mRenderer.DestroyMesh(mesh);
    }
    for(auto pair : mTextureManager.mTextureMap)
    {
      Texture* texture = pair.second;
      mRenderer.DestroyTexture(texture);
    }

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
      vkDestroyFence(mDevice, mSyncObjects.mInFlightFences[i], nullptr);
      vkDestroySemaphore(mDevice, mSyncObjects.mRenderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(mDevice, mSyncObjects.mImageAvailableSemaphores[i], nullptr);
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

  VkRenderPass mRenderPass;
  VkPipelineLayout mPipelineLayout;
  VkPipeline mGraphicsPipeline;
  VkCommandPool mCommandPool;
  VkDescriptorPool mDescriptorPool;
  std::vector<VkDescriptorSet> mDescriptorSets;

  std::vector<VkFramebuffer> mSwapChainFramebuffers;
  std::vector<VkCommandBuffer> mCommandBuffers;

  SyncObjects mSyncObjects;
  size_t mCurrentFrame = 0;

  bool mFramebufferResized = false;

  std::vector<Model*> mModels;

  VulkanUniformBuffers mUniformBuffers;
  VulkanUniformBuffer mMaterialBuffer;

  MeshManager mMeshManager;
  TextureManager mTextureManager;
  ShaderManager mShaderManager;
  MaterialManager mMaterialManager;
  VulkanRenderer mRenderer;
  std::vector<MaterialDescriptorSetLayout> mGlobalDescriptorLayouts;
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