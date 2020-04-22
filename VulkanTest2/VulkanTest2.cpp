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
#include <stdexcept>
#include <functional>
#include <optional>
#include <algorithm>
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
#include "Helpers/Shader.hpp"
#include "Helpers/Material.hpp"
#include "Helpers/MaterialBinding.hpp"
#include "Helpers/Model.hpp"
#include "Helpers/Texture.hpp"
#include "VulkanStructures.hpp"
#include "VulkanRenderer.hpp"

#include "JsonSerializers.hpp"

const int cWidth = 800;
const int cHeight = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;
const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct PerCameraData {
  alignas(16) Matrix4 view;
  alignas(16) Matrix4 proj;
};

struct PerObjectData {
  alignas(16) Matrix4 model;
};


#pragma optimize("", off)

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
  }

  void LoadResources()
  {
    mTextureManager.Load();
    mShaderManager.Load();
    mMaterialManager.Load();
    mMeshManager.Load();
    LoadLevel("Level");
    LoadShadersAndMaterials();
  }

  void LoadVulkanResources()
  {
    LoadVulkanImages();
    LoadVulkanShaders();
    LoadVulkanMaterials();
    LoadVulkanMeshes();
  }

  void LoadShadersAndMaterials()
  {
    for(auto pair : mShaderManager.mShaderMap)
    {
      String shaderName = pair.first;
      Shader* shader = pair.second;

      ShaderBinding& shaderBinding = mShaderBindings[shaderName];
      shaderBinding.AddBinding("PerCameraData", MaterialDescriptorType::Uniform, ShaderMaterialBindingId::Global);
      shaderBinding.AddBinding("PerObjectData", MaterialDescriptorType::UniformDynamic, ShaderMaterialBindingId::Global);
      shaderBinding.Initialize(shader, MaterialDescriptorType::Uniform, ShaderMaterialBindingId::Material);
      shaderBinding.CompileBindings();
    }

    for(auto pair : mMaterialManager.mMaterialMap)
    {
      String materialName = pair.first;
      Material* material = pair.second;

      Shader* shader = mShaderManager.Find(material->mShaderName);
      ShaderBinding& shaderBinding = mShaderBindings[material->mShaderName];
      ShaderMaterialBinding& binding = mShaderMaterialBindings[shader];
      binding.CompileBindings(shaderBinding, *material);
    }
  }

  void LoadLevel(const String& levelName)
  {
    String filePath = String("data/") + levelName + String(".level");
    JsonLoader loader;
    loader.LoadFromFile(filePath);

    size_t objCount;
    loader.BeginArray(objCount);
    for(size_t objIndex = 0; objIndex < objCount; ++objIndex)
    {
      loader.BeginArrayItem(objIndex);

      Model* model = new Model();
      LoadModel(loader, model);
      mModels.emplace_back(model);

      loader.EndArrayItem();
    }
  }

  void LoadVulkanMeshes()
  {
    for(auto pair : mMeshManager.mMeshMap)
    {
      Mesh* mesh = pair.second;
      mRenderer.CreateMesh(mesh);
    }
  }

  void LoadVulkanImages()
  {
    for(auto pair : mTextureManager.mTextureMap)
    {
      mRenderer.CreateTexture(pair.second);
    }
  }

  void LoadVulkanShaders()
  {
    for(auto pair : mShaderManager.mShaderMap)
    {
      Shader* shader = pair.second;
      mRenderer.CreateShader(shader);
      mRenderer.CreateShaderMaterial(&mShaderBindings[pair.first]);
    }
  }

  void LoadVulkanMaterial(Material* material)
  {
    Shader* shader = mShaderManager.Find(material->mShaderName);
    ShaderMaterialBinding& shaderMaterial = mShaderMaterialBindings[shader];

    mRenderer.UpdateShaderMaterial(&mShaderMaterialBindings[shader]);

    uint32_t offset = 0;
    for(auto pair : shaderMaterial.mShaderBinding->mBindings)
    {
      ShaderResourceBinding* shaderBinding = pair.second;
      if(shaderBinding->mMaterialBindingId == ShaderMaterialBindingId::Material)
      {
        shaderBinding->mBufferOffset = offset;
        offset += static_cast<uint32_t>(shaderBinding->mBoundResource->mSizeInBytes);
      }
    }
  }

  void LoadVulkanMaterials()
  {
    for(auto pair : mMaterialManager.mMaterialMap)
    {
      LoadVulkanMaterial(pair.second);
    }
  }

  void PopulateMaterialBuffer()
  {
    struct BufferSortData
    {
      uint32_t mBufferId;
      MaterialProperty* mProperty;
      ShaderFieldBinding* mFieldBinding;
    };
    auto sortLambda = [](const BufferSortData& rhs, const BufferSortData& lhs)
    {
      return rhs.mBufferId < lhs.mBufferId;
    };
    std::vector<BufferSortData> propertiesByBuffer;

    for(auto pair : mMaterialManager.mMaterialMap)
    {
      Material* material = pair.second;
      Shader* shader = mShaderManager.Find(material->mShaderName);
      ShaderMaterialBinding& shaderMaterial = mShaderMaterialBindings[shader];
      VulkanShaderMaterial* vulkanShaderMaterial = mRenderer.mShaderMaterialMap[shaderMaterial.mShaderBinding];

      for(MaterialProperty& materialProp : material->mProperties)
      {
        auto it = shaderMaterial.mMaterialNameMap.find(materialProp.mPropertyName);
        if(it == shaderMaterial.mMaterialNameMap.end())
          continue;

        ShaderFieldBinding* fieldBinding = it->second;
        const ShaderResourceField* fieldResource = fieldBinding->mShaderField;
        if(fieldResource != nullptr)
        {
          BufferSortData sortData{vulkanShaderMaterial->mBufferId, &materialProp, fieldBinding};
          propertiesByBuffer.emplace_back(sortData);
        }
      }
    }

    uint32_t bufferId = static_cast<uint32_t>(-1);
    VkDeviceMemory bufferMemory = VK_NULL_HANDLE;
    byte* byteData = nullptr;
    for(size_t i = 0; i < propertiesByBuffer.size(); ++i)
    {
      BufferSortData& data = propertiesByBuffer[i];
      if(bufferId != data.mBufferId || byteData == nullptr)
      {
        if(byteData != nullptr)
          mRenderer.UnMapUniformBufferMemory(UniformBufferType::Material, bufferId);

        bufferId = data.mBufferId;
        byteData = static_cast<byte*>(mRenderer.MapUniformBufferMemory(UniformBufferType::Material, bufferId));
      }

      MaterialProperty* prop = data.mProperty;
      unsigned char* fieldStart = byteData + data.mFieldBinding->mShaderField->mOffset + data.mFieldBinding->mOwningBinding->mBufferOffset;
      // This might be wrong due to stride, have to figure out how to deal with this...
      memcpy(fieldStart, prop->mData.data(), prop->mData.size());
    }
  }

  void initVulkan()
  {
    GlobalSetup();

    LoadResources();
    LoadVulkanResources();
    PopulateMaterialBuffer();
  }

  void cleanupSwapChain()
  {
    mRenderer.DestroyUniformBuffer(0);
    
    for(auto pair : mShaderMaterialBindings)
    {
      ShaderMaterialBinding& shaderMaterial = pair.second;
      mRenderer.DestroyShaderMaterial(shaderMaterial.mShaderBinding);
    }
    mRenderer.DestroyRenderFramesInternal();
    mRenderer.DestroySwapChainInternal();
    mRenderer.DestroyDepthResourcesInternal();
  }

  void recreateSwapChain()
  {
    int width = 0, height = 0;
    while(width == 0 || height == 0)
    {
      glfwGetFramebufferSize(mWindow, &width, &height);
      glfwWaitEvents();
    }

    vkDeviceWaitIdle(mRenderer.mInternal->mDevice);

    cleanupSwapChain();
    mRenderer.Resize(width, height);
    mRenderer.CreateDepthResourcesInternal();
    mRenderer.CreateSwapChainInternal();
    mRenderer.CreateRenderFramesInternal();

    for(auto pair : mShaderMaterialBindings)
    {
      ShaderMaterialBinding& shaderMaterial = pair.second;
      mRenderer.CreateShaderMaterial(shaderMaterial.mShaderBinding);
      mRenderer.UpdateShaderMaterial(&shaderMaterial);
    }
  }

  

  

  size_t AlignUniformBufferOffset(size_t offset)
  {
    return ::AlignUniformBufferOffset(mRenderer.mInternal->mDeviceLimits, offset);
  }

  struct FrameData
  {
    VkFramebuffer mFramebuffer;
    VkCommandBuffer mCommandBuffer;
    uint32_t mIndex;
  };

  void prepareFrame(FrameData& frameData)
  {
    UpdateGlobalBuffer(frameData);

    VkCommandBuffer commandBuffer = frameData.mCommandBuffer;
    
    uint32_t dynamicOffsets[1] = 
    {
      static_cast<uint32_t>(AlignUniformBufferOffset(sizeof(PerObjectData)))
      
    };
    
    uint32_t dynamicOffsetBase[1] = {0};
    CommandBufferWriteInfo writeInfo;
    writeInfo.mDevice = mRenderer.mInternal->mDevice;
    writeInfo.mCommandPool = mRenderer.mInternal->mCommandPool;
    writeInfo.mRenderPass = mRenderer.mInternal->mRenderFrames[0].mRenderPass;
    //writeInfo.mGraphicsPipeline = materialPipeline.mPipeline;
    //writeInfo.mPipelineLayout = materialPipeline.mPipelineLayout;
    //writeInfo.mVertexBuffer = mesh->mVertexBuffer;
    //writeInfo.mIndexBuffer = mesh->mIndexBuffer;
    //writeInfo.mIndexBufferCount = mesh->mIndexCount;
    //writeInfo.mDescriptorSet = frameData.mDescriptorSet;
    writeInfo.mSwapChain = mRenderer.mInternal->mSwapChain.mSwapChain;
    writeInfo.mSwapChainExtent = mRenderer.mInternal->mSwapChain.mExtent;
    writeInfo.mSwapChainFramebuffer = frameData.mFramebuffer;
    writeInfo.mDrawCount = static_cast<uint32_t>(mModels.size());
    writeInfo.mDynamicOffsetsCount = 1;
    writeInfo.mDynamicOffsetsBase = dynamicOffsetBase;
    writeInfo.mDynamicOffsets = dynamicOffsets;

    BeginCommandBuffer(commandBuffer);

    BeginRenderPass(writeInfo, commandBuffer);

    for(size_t i = 0; i < mModels.size(); ++i)
    {
      VulkanMesh* vulkanMesh = mRenderer.mMeshMap[mMeshManager.Find(mModels[i]->mMeshName)];
      Material* material = mMaterialManager.Find(mModels[i]->mMaterialName);
      VulkanShaderMaterial* vulkanShaderMaterial = GetVulkanShaderMaterial(material);
      //VulkanMaterialPipeline& materialPipeline = *GetMaterialPipeline(material);
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanShaderMaterial->mPipeline);

      VkBuffer vertexBuffers[] = {vulkanMesh->mVertexBuffer};
      VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
      vkCmdBindIndexBuffer(commandBuffer, vulkanMesh->mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanShaderMaterial->mPipelineLayout, 0, 1, &vulkanShaderMaterial->mDescriptorSets[i], writeInfo.mDynamicOffsetsCount, writeInfo.mDynamicOffsetsBase);
      vkCmdDrawIndexed(commandBuffer, vulkanMesh->mIndexCount, 1, 0, 0, 0);

      for(size_t j = 0; j < writeInfo.mDynamicOffsetsCount; ++j)
        writeInfo.mDynamicOffsetsBase[j] += writeInfo.mDynamicOffsets[j];
    }

    EndRenderPass(writeInfo, commandBuffer);

    EndCommandBuffer(commandBuffer);
  }

  void UpdateGlobalBuffer(FrameData& frameData)
  {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    float nearDistance = 0.1f;
    float farDistance = 10.0f;
    VkExtent2D extent = mRenderer.mInternal->mSwapChain.mExtent;
    float aspectRatio = extent.width / (float)extent.height;
    float fov = glm::radians(45.0f);
    auto lookAt = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    PerCameraData perCameraData;
    perCameraData.view.Load(&lookAt[0][0]);
    perCameraData.proj = mRenderer.BuildPerspectiveMatrix(fov, aspectRatio, nearDistance, farDistance);

    byte* data = static_cast<byte*>(mRenderer.MapUniformBufferMemory(UniformBufferType::Global, 0, frameData.mIndex));

    size_t offset = 0;
    memcpy(data, &perCameraData, sizeof(perCameraData));
    offset += AlignUniformBufferOffset(sizeof(perCameraData));

    size_t count = mModels.size();
    for(size_t i = 0; i < count; ++i)
    {
      Model* model = mModels[i];
      glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(model->mScale.x, model->mScale.y, model->mScale.z));
      glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
      glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(model->mTranslation.x, model->mTranslation.y, model->mTranslation.z));
      glm::mat4 transform = translation * rotation * scale;
      byte* memory = data + offset + AlignUniformBufferOffset(sizeof(PerObjectData)) * i;
      memcpy(memory, &transform, sizeof(transform));
    }
    mRenderer.UnMapUniformBufferMemory(UniformBufferType::Global, 0, frameData.mIndex);
  }

  void mainLoop()
  {
    while(!glfwWindowShouldClose(mWindow))
    {
      glfwPollEvents();
      drawFrame();
    }

    vkDeviceWaitIdle(mRenderer.mInternal->mDevice);
  }

  void drawFrame()
  {
    RenderFrame* renderFrame = nullptr;
    RenderFrameStatus status = mRenderer.BeginFrame(renderFrame);
    if(status == RenderFrameStatus::OutOfDate)
    {
      mFramebufferResized = false;
      recreateSwapChain();
      return;
    }

    uint32_t imageIndex = renderFrame->mId;
    auto& uniformBuffers = *mRenderer.RequestUniformBuffer(0);
    FrameData frameData;
    frameData.mIndex = imageIndex;
    frameData.mCommandBuffer = mRenderer.mInternal->mRenderFrames[imageIndex].mCommandBuffer;
    frameData.mFramebuffer = mRenderer.mInternal->mRenderFrames[imageIndex].mFrameBuffer;
    prepareFrame(frameData);


    status = mRenderer.EndFrame(renderFrame);
    if(status == RenderFrameStatus::OutOfDate || status == RenderFrameStatus::SubOptimal)
      recreateSwapChain();
    else if(status != RenderFrameStatus::Success)
      throw std::runtime_error("failed to present swap chain image!");
  }

  void cleanup() 
  {
    cleanupSwapChain();
    //CleanupUniform(GetMaterialBuffer());

    //for(auto pair : mShaderBindings)
    //{
    //  ShaderBinding& shaderBinding = pair.second;
    //  mRenderer.DestroyShaderMaterial(&shaderBinding);
    //}
    for(auto pair : mShaderManager.mShaderMap)
    {
      Shader* shader= pair.second;
      mRenderer.DestroyShader(shader);
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
    mRenderer.CleanupResources();
    mRenderer.Shutdown();

    if(enableValidationLayers)
      DestroyDebugUtilsMessengerEXT(mRenderer.mInternal->mInstance, mRenderer.mInternal->mDebugMessenger, nullptr);

    vkDestroyCommandPool(mRenderer.mInternal->mDevice, mRenderer.mInternal->mCommandPool, nullptr);

    vkDestroyDevice(mRenderer.mInternal->mDevice, nullptr);
    vkDestroySurfaceKHR(mRenderer.mInternal->mInstance, mRenderer.mInternal->mSurface, nullptr);
    vkDestroyInstance(mRenderer.mInternal->mInstance, nullptr);
    glfwDestroyWindow(mWindow);
    glfwTerminate();
  }

  VulkanShader* GetVulkanShader(Material* material)
  {
    Shader* shader = mShaderManager.Find(material->mShaderName);
    return mRenderer.mShaderMap[shader];
  }
  VulkanShaderMaterial* GetVulkanShaderMaterial(Material* material)
  {
    ShaderBinding& shaderBinding = mShaderBindings[material->mShaderName];
    return mRenderer.mShaderMaterialMap[&shaderBinding];
  }

  GLFWwindow* mWindow;

  bool mFramebufferResized = false;

  std::vector<Model*> mModels;

  MeshManager mMeshManager;
  TextureManager mTextureManager;
  ShaderManager mShaderManager;
  MaterialManager mMaterialManager;
  VulkanRenderer mRenderer;
  std::unordered_map<String, ShaderBinding> mShaderBindings;
  std::unordered_map<Shader*, ShaderMaterialBinding> mShaderMaterialBindings;
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