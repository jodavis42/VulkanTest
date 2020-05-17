#include "Precompiled.hpp"

#include "GraphicsSpace.hpp"
#include "GraphicsEngine.hpp"
#include "GraphicsBufferTypes.hpp"

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
  PrepareAndDrawFrame(*renderFrame);
  
  status = renderer.EndFrame(renderFrame);
  if(status == RenderFrameStatus::OutOfDate || status == RenderFrameStatus::SubOptimal)
    mEngine->RecreateSwapChain();
  else if(status != RenderFrameStatus::Success)
  {
    ErrorIf(true, "failed to present swap chain image!");
  }
}

void GraphicsSpace::PrepareAndDrawFrame(RenderFrame& renderFrame)
{
  VulkanRenderer& renderer = mEngine->mRenderer;

  // Batch up all model draw calls
  size_t count = mModels.Size();
  for(size_t i = 0; i < count; ++i)
  {
    const Model* model = mModels[i];
    Matrix3 rotation = Matrix3::GenerateRotation(Vec3(0, 0, 1), 0 * Math::DegToRad(90.0f));
    Matrix4 transform = Matrix4::GenerateTransform(model->mTranslation, rotation, model->mScale);

    ModelRenderData modelRenderData;
    modelRenderData.mModel = model;
    modelRenderData.mMesh = mEngine->mMeshManager.Find(model->mMeshName);
    modelRenderData.mZilchShader = mEngine->mZilchShaderManager.Find(model->mMaterialName);
    modelRenderData.mZilchMaterial = mEngine->mZilchMaterialManager.Find(model->mMaterialName);
    modelRenderData.mTransform = transform;
    renderer.QueueDraw(modelRenderData);
  }

  // Create the batch draw call for the frame
  RenderBatchDrawData batchDrawData;

  float nearDistance = 0.1f;
  float farDistance = 10.0f;
  size_t width, height;
  renderer.GetSize(width, height);
  float aspectRatio = width / (float)height;
  float fov = Math::DegToRad(45.0f);

  FrameData& frameData = batchDrawData.mFrameData;
  frameData.mFrameTime = mTotalTimeElapsed;
  frameData.mLogicTime = mTotalTimeElapsed;

  CameraData& cameraData = batchDrawData.mCameraData;
  cameraData.mFarPlane = 1;
  cameraData.mNearPlane = 0;
  cameraData.mViewportSize = Vec2::cZero;

  batchDrawData.mWorldToView = GenerateLookAt(Vec3(5.0f, 5.0f, 5.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
  batchDrawData.mViewToPerspective = renderer.BuildPerspectiveMatrix(fov, aspectRatio, nearDistance, farDistance);
  batchDrawData.mWorldToView.Transpose();
  batchDrawData.mViewToPerspective.Transpose();
  renderer.Draw(batchDrawData);
}
