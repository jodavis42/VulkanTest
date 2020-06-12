#include "Precompiled.hpp"

#include "EngineZilchStaticLibrary.hpp"
#include "ArchetypeManager.hpp"
#include "Component.hpp"
#include "Composition.hpp"
#include "Engine.hpp"
#include "GameSession.hpp"
#include "Keyboard.hpp"
#include "LevelManager.hpp"
#include "Mouse.hpp"
#include "UpdateEvent.hpp"
#include "TimeSpace.hpp"
#include "Transform.hpp"
#include "Space.hpp"

// Get a component from a Composition in zilch
void GetComponentExtensionProperty(Zilch::Call& call, Zilch::ExceptionReport& report)
{
  Zilch::Function* currentFunction = call.GetFunction();
  Zilch::BoundType* boundType = Zilch::Type::DynamicCast<Zilch::BoundType*>(currentFunction->FunctionType->Return);

  Zilch::Handle& selfHandle = call.GetHandle(Zilch::Call::This);
  Composition* composition = selfHandle.Get<Composition*>();

  Zilch::Handle componentInstance = composition->FindComponent(boundType);
  call.Set(Zilch::Call::Return, componentInstance);
}

void AddComponentExtensions(Zilch::LibraryBuilder& builder, Zilch::BoundType* boundType)
{
  if(boundType->IsA(ZilchTypeId(Component)))
    builder.AddExtensionGetterSetter(ZilchTypeId(Composition), boundType->Name, boundType, nullptr, GetComponentExtensionProperty, Zilch::MemberOptions::None);
}

void AddNativeLibraryExtensions(Zilch::LibraryBuilder& builder)
{
  for(auto range = builder.BoundTypes.Values(); !range.Empty(); range.PopFront())
    AddComponentExtensions(builder, range.Front());
}

ZilchDefineExternalBaseType(Keys::Enum, Zilch::TypeCopyMode::ValueType, builder, type)
{
  ZilchFullBindEnum(builder, type, Zilch::SpecialType::Enumeration);
  
  ZilchFullBindEnumValue(builder, type, Keys::Num0, "Num0");
  ZilchFullBindEnumValue(builder, type, Keys::Num1, "Num1");
  ZilchFullBindEnumValue(builder, type, Keys::Num2, "Num2");
  ZilchFullBindEnumValue(builder, type, Keys::Num3, "Num3");
  ZilchFullBindEnumValue(builder, type, Keys::Num4, "Num4");
  ZilchFullBindEnumValue(builder, type, Keys::Num5, "Num5");
  ZilchFullBindEnumValue(builder, type, Keys::Num6, "Num6");
  ZilchFullBindEnumValue(builder, type, Keys::Num7, "Num7");
  ZilchFullBindEnumValue(builder, type, Keys::Num8, "Num8");
  ZilchFullBindEnumValue(builder, type, Keys::Num9, "Num9");
  ZilchFullBindEnumValue(builder, type, Keys::A, "A");
  ZilchFullBindEnumValue(builder, type, Keys::B, "B");
  ZilchFullBindEnumValue(builder, type, Keys::C, "C");
  ZilchFullBindEnumValue(builder, type, Keys::D, "D");
  ZilchFullBindEnumValue(builder, type, Keys::E, "E");
  ZilchFullBindEnumValue(builder, type, Keys::F, "F");
  ZilchFullBindEnumValue(builder, type, Keys::G, "G");
  ZilchFullBindEnumValue(builder, type, Keys::H, "H");
  ZilchFullBindEnumValue(builder, type, Keys::I, "I");
  ZilchFullBindEnumValue(builder, type, Keys::J, "J");
  ZilchFullBindEnumValue(builder, type, Keys::K, "K");
  ZilchFullBindEnumValue(builder, type, Keys::L, "L");
  ZilchFullBindEnumValue(builder, type, Keys::M, "M");
  ZilchFullBindEnumValue(builder, type, Keys::N, "N");
  ZilchFullBindEnumValue(builder, type, Keys::O, "O");
  ZilchFullBindEnumValue(builder, type, Keys::P, "P");
  ZilchFullBindEnumValue(builder, type, Keys::Q, "Q");
  ZilchFullBindEnumValue(builder, type, Keys::R, "R");
  ZilchFullBindEnumValue(builder, type, Keys::S, "S");
  ZilchFullBindEnumValue(builder, type, Keys::T, "T");
  ZilchFullBindEnumValue(builder, type, Keys::U, "U");
  ZilchFullBindEnumValue(builder, type, Keys::V, "V");
  ZilchFullBindEnumValue(builder, type, Keys::W, "W");
  ZilchFullBindEnumValue(builder, type, Keys::X, "X");
  ZilchFullBindEnumValue(builder, type, Keys::Y, "Y");
  ZilchFullBindEnumValue(builder, type, Keys::Z, "Z");
  ZilchFullBindEnumValue(builder, type, Keys::Tilde, "Tilde");
  ZilchFullBindEnumValue(builder, type, Keys::Comma, "Comma");
  ZilchFullBindEnumValue(builder, type, Keys::Period, "Period");
  ZilchFullBindEnumValue(builder, type, Keys::LeftBracket, "LeftBracket");
  ZilchFullBindEnumValue(builder, type, Keys::RightBracket, "RightBracket");
  ZilchFullBindEnumValue(builder, type, Keys::Semicolon, "Semicolon");
  ZilchFullBindEnumValue(builder, type, Keys::Up, "Up");
  ZilchFullBindEnumValue(builder, type, Keys::Down, "Down");
  ZilchFullBindEnumValue(builder, type, Keys::Left, "Left");
  ZilchFullBindEnumValue(builder, type, Keys::Right, "Right");
  ZilchFullBindEnumValue(builder, type, Keys::Space, "Space");
  ZilchFullBindEnumValue(builder, type, Keys::Tab, "Tab");
  ZilchFullBindEnumValue(builder, type, Keys::Enter, "Enter");
  ZilchFullBindEnumValue(builder, type, Keys::Shift, "Shift");
  ZilchFullBindEnumValue(builder, type, Keys::Control, "Control");
  ZilchFullBindEnumValue(builder, type, Keys::Alt, "Alt");
}

ZilchDefineExternalBaseType(MouseButtons::Enum, Zilch::TypeCopyMode::ValueType, builder, type)
{
  ZilchFullBindEnum(builder, type, Zilch::SpecialType::Enumeration);

  ZilchFullBindEnumValue(builder, type, MouseButtons::Left, "Left");
  ZilchFullBindEnumValue(builder, type, MouseButtons::Middle, "Middle");
  ZilchFullBindEnumValue(builder, type, MouseButtons::Right, "Right");
}

ZilchDefineExternalBaseType(MouseButtonStates::Enum, Zilch::TypeCopyMode::ValueType, builder, type)
{
  ZilchFullBindEnum(builder, type, Zilch::SpecialType::Enumeration);

  ZilchFullBindEnumValue(builder, type, MouseButtonStates::Up, "Up");
  ZilchFullBindEnumValue(builder, type, MouseButtonStates::Down, "Down");
}

ZilchDefineStaticLibrary(EngineStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Enums
  ZilchInitializeExternalTypeAs(Keys::Enum, "Keys");
  ZilchInitializeExternalTypeAs(MouseButtons::Enum, "MouseButtons");
  ZilchInitializeExternalTypeAs(MouseButtonStates::Enum, "MouseButtonStates");

  // Events
  ZilchInitializeType(UpdateEvent);
  ZilchInitializeType(KeyboardEvent);
  ZilchInitializeType(MouseEvent);

  // Resources
  ZilchInitializeType(Level);
  ZilchInitializeType(Archetype);

  // Compositions
  ZilchInitializeType(Composition);
  ZilchInitializeType(Space);
  ZilchInitializeType(GameSession);
  ZilchInitializeType(Engine);

  // Components
  ZilchInitializeType(Component);
  ZilchInitializeType(Keyboard);
  ZilchInitializeType(Mouse);
  ZilchInitializeType(TimeSpace);
  ZilchInitializeType(Transform);

  AddNativeLibraryExtensions(builder);
}
