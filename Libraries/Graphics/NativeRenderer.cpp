#include "Precompiled.hpp"

#include "NativeRenderer.hpp"
#include "RenderTasks.hpp"
#include "Space.hpp"
#include "Engine.hpp"
#include "GraphicsSpace.hpp"
#include "GraphicsEngine.hpp"

//-------------------------------------------------------------------NativeRenderer
ZilchDefineType(NativeRenderer, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindFieldProperty(mActive);
}

void NativeRenderer::Initialize(const CompositionInitializer& initializer)
{
  Zilch::EventConnect(GetSpace(), Events::CollectRenderTasks, &NativeRenderer::OnCollectRenderTasks, this);
}

void NativeRenderer::OnCollectRenderTasks(RenderTaskEvent* renderTaskEvent)
{
  if(!mActive)
    return;

  Space* space = GetSpace();
  GraphicsSpace* graphicsSpace = space->Has<GraphicsSpace>();
  GraphicsEngine* graphicsEngine = graphicsSpace->mEngine;
  ResourceSystem* resourceSystem = graphicsEngine->mResourceSystem;
  Engine* engine = GetEngine();

  renderTaskEvent->CreateClearTargetRenderTask();
  RenderGroupRenderTask* renderGroupTask = renderTaskEvent->CreateRenderGroupRenderTask();
  RenderGroupManager* renderGroupManager = resourceSystem->FindResourceManager(RenderGroupManager);
  RenderGroup* opaqueRenderGroup = renderGroupManager->FindResource(ResourceName{"Opaque"});
  renderGroupTask->AddRenderGroup(opaqueRenderGroup);
}
