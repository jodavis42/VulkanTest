#include "Precompiled.hpp"

#include "ZilchComponent.hpp"
#include "Composition.hpp"
#include "Space.hpp"

//-----------------------------------------------------------------------------ZilchComponent
ZilchDefineType(ZilchComponent, builder, type)
{
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();
  type->Sealed = false;
}

ZilchComponent::~ZilchComponent()
{

}

void ZilchComponent::Initialize(const CompositionInitializer& initializer)
{
  Zilch::EventConnect(GetSpace(), "LogicUpdate", &ZilchComponent::OnLogicUpdate, this);
  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::BoundType* thisType = ZilchVirtualTypeId(this);
  Array<Zilch::Type*> params;
  static String FunctionName("Initialize");
  Zilch::Function* function = thisType->FindFunction(FunctionName, params, core.VoidType, Zilch::FindMemberOptions::None);
  if(function == nullptr)
    return;

  Zilch::ExceptionReport report;
  Zilch::Call call(function);

  call.SetHandle(Zilch::Call::This, this);
  call.Invoke(report);
}

void ZilchComponent::OnLogicUpdate(Zilch::EventData* event)
{
  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::BoundType* thisType = ZilchVirtualTypeId(this);
  Array<Zilch::Type*> params;
  params.PushBack(core.RealType);
  static String FunctionName("Update");
  Zilch::Function* function = thisType->FindFunction(FunctionName, params, core.VoidType, Zilch::FindMemberOptions::None);
  if(function == nullptr)
    return;

  Zilch::ExceptionReport report;
  Zilch::Call call(function);

  call.SetHandle(Zilch::Call::This, this);
  call.SetValue(0, 1 / 60.0f);
  call.Invoke(report);
}

