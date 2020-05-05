#include "Precompiled.hpp"

#include "GraphicsSpace.hpp"
#include "GraphicsEngine.hpp"

#include "VulkanStructures.hpp"
#include "VulkanInitialization.hpp"
#include "VulkanCommandBuffer.hpp"

static Matrix4 GenerateLookAt(const Vec3& eye, const Vec3& center, const Vec3& worldUp)
{
  Vec3 forward = Vec3::Normalized(center - eye);
  Vec3 right = Vec3::Normalized(Vec3::Cross(forward, worldUp));
  Vec3 actualUp = Vec3::Cross(right, forward);

  Matrix4 result;
  result.SetIdentity();
  result.m00 = right.x;
  result.m10 = right.y;
  result.m20 = right.z;
  result.m01 = actualUp.x;
  result.m11 = actualUp.y;
  result.m21 = actualUp.z;
  result.m02 = -forward.x;
  result.m12 = -forward.y;
  result.m22 = -forward.z;
  result.m30 = -Vec3::Dot(right, eye);
  result.m31 = -Vec3::Dot(actualUp, eye);
  result.m32 = Vec3::Dot(forward, eye);
  return result;
}

void GraphicsSpace::Update(UpdateEvent& e)
{
  mTotalTimeElapsed += e.mDt;
}

void GraphicsSpace::UpdateGlobalBuffer(uint32_t frameId)
{
  VulkanRenderer* renderer = &mEngine->mRenderer;

  float nearDistance = 0.1f;
  float farDistance = 10.0f;
  VkExtent2D extent = renderer->mInternal->mSwapChain.mExtent;
  float aspectRatio = extent.width / (float)extent.height;
  float fov = Math::DegToRad(45.0f);

  PerCameraData perCameraData;
  perCameraData.view = GenerateLookAt(Vec3(5.0f, 5.0f, 5.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
  perCameraData.proj = renderer->BuildPerspectiveMatrix(fov, aspectRatio, nearDistance, farDistance);

  byte* data = static_cast<byte*>(renderer->MapUniformBufferMemory(UniformBufferType::Global, 0, frameId));

  size_t offset = 0;
  memcpy(data, &perCameraData, sizeof(perCameraData));
  offset += renderer->AlignUniformBufferOffset(sizeof(perCameraData));

  size_t count = mModels.Size();
  for(size_t i = 0; i < count; ++i)
  {
    Model* model = mModels[i];
    Matrix3 rotation = Matrix3::GenerateRotation(Vec3(0, 0, 1), mTotalTimeElapsed * Math::DegToRad(90.0f));
    Matrix4 transform = Matrix4::GenerateTransform(model->mTranslation, rotation, model->mScale);
    transform = Matrix4::Transposed(transform);
    byte* memory = data + offset + renderer->AlignUniformBufferOffset(sizeof(PerObjectData)) * i;
    memcpy(memory, &transform, sizeof(transform));
  }
  renderer->UnMapUniformBufferMemory(UniformBufferType::Global, 0, frameId);
}

void GraphicsSpace::Draw(UpdateEvent& toSend)
{
  VulkanRenderer& renderer = mEngine->mRenderer;
  RenderFrame* renderFrame = nullptr;
  RenderFrameStatus status = renderer.BeginFrame(renderFrame);
  if(status == RenderFrameStatus::OutOfDate)
  {
    mEngine->RecreateSwapChain();
    return;
  }

  uint32_t imageIndex = renderFrame->mId;
  Update(toSend);
  UpdateGlobalBuffer(imageIndex);
  PrepareFrame(*renderFrame);
  
  
  status = renderer.EndFrame(renderFrame);
  if(status == RenderFrameStatus::OutOfDate || status == RenderFrameStatus::SubOptimal)
    mEngine->RecreateSwapChain();
  else if(status != RenderFrameStatus::Success)
    throw std::runtime_error("failed to present swap chain image!");
}

VulkanShaderMaterial* GetVulkanShaderMaterial(GraphicsSpace* space, Material* material)
{
  UniqueShaderMaterial& uniqueShaderMaterial = space->mEngine->mUniqueShaderMaterialNameMap[material->mShaderName];
  return space->mEngine->mRenderer.mUniqueShaderMaterialMap[&uniqueShaderMaterial];
}

void GraphicsSpace::PrepareFrame(RenderFrame& renderFrame)
{
  uint32_t imageIndex = renderFrame.mId;
  VulkanRenderer& renderer = mEngine->mRenderer;
  VulkanRenderFrame& vulkanRenderFrame = renderer.mInternal->mRenderFrames[imageIndex];
  VkCommandBuffer commandBuffer = vulkanRenderFrame.mCommandBuffer;


  uint32_t dynamicOffsets[1] =
  {
    static_cast<uint32_t>(renderer.AlignUniformBufferOffset(sizeof(PerObjectData)))

  };

  uint32_t dynamicOffsetBase[1] = {0};
  CommandBufferWriteInfo writeInfo;
  writeInfo.mDevice = renderer.mInternal->mDevice;
  writeInfo.mCommandPool = renderer.mInternal->mCommandPool;
  writeInfo.mRenderPass = renderer.mInternal->mRenderFrames[0].mRenderPass;
  //writeInfo.mGraphicsPipeline = materialPipeline.mPipeline;
  //writeInfo.mPipelineLayout = materialPipeline.mPipelineLayout;
  //writeInfo.mVertexBuffer = mesh->mVertexBuffer;
  //writeInfo.mIndexBuffer = mesh->mIndexBuffer;
  //writeInfo.mIndexBufferCount = mesh->mIndexCount;
  //writeInfo.mDescriptorSet = frameData.mDescriptorSet;
  writeInfo.mSwapChain = renderer.mInternal->mSwapChain.mSwapChain;
  writeInfo.mSwapChainExtent = renderer.mInternal->mSwapChain.mExtent;
  writeInfo.mSwapChainFramebuffer = vulkanRenderFrame.mFrameBuffer;
  writeInfo.mDrawCount = static_cast<uint32_t>(mModels.Size());
  writeInfo.mDynamicOffsetsCount = 1;
  writeInfo.mDynamicOffsetsBase = dynamicOffsetBase;
  writeInfo.mDynamicOffsets = dynamicOffsets;

  BeginCommandBuffer(commandBuffer);

  BeginRenderPass(writeInfo, commandBuffer);

  for(size_t i = 0; i < mModels.Size(); ++i)
  {
    VulkanMesh* vulkanMesh = renderer.mMeshMap[mEngine->mMeshManager.Find(mModels[i]->mMeshName)];
    Material* material = mEngine->mMaterialManager.Find(mModels[i]->mMaterialName);
    VulkanShaderMaterial* vulkanShaderMaterial = GetVulkanShaderMaterial(this, material);
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
