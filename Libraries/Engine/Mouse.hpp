///////////////////////////////////////////////////////////////////////////////
/// Authors: Joshua Davis
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "EngineStandard.hpp"

#include "Component.hpp"

namespace MouseButtons { enum Enum { None, Left, Middle, Right, Count }; };
namespace MouseButtonStates {enum Enum { None, Down, Up};};

class Mouse;

//-----------------------------------------------------------------------------MouseButton
class MouseButton
{
public:
  MouseButton();
  MouseButtonStates::Enum mState;
};

//-----------------------------------------------------------------------------MouseEvent
class MouseEvent : public Zilch::EventData
{
public:
  ZilchDeclareType(MouseEvent, Zilch::TypeCopyMode::ReferenceType);
  MouseEvent();

  /// The button that triggered this event.
  MouseButtons::Enum GetButton() const;
  /// The current state of the button
  bool GetIsDown() const;

  MouseButtons::Enum mButton;
  MouseButtonStates::Enum mState;
  Vec2 mMovement;
  Vec2 mPosition;
  Vec2 mScroll;
  Mouse* mMouse = nullptr;
};

namespace Events
{
ZilchDeclareEvent(LeftMouseDown, MouseEvent);
ZilchDeclareEvent(LeftMouseUp, MouseEvent);
ZilchDeclareEvent(RightMouseDown, MouseEvent);
ZilchDeclareEvent(RightMouseUp, MouseEvent);
ZilchDeclareEvent(MiddleMouseDown, MouseEvent);
ZilchDeclareEvent(MiddleMouseUp, MouseEvent);
ZilchDeclareEvent(MouseMove, MouseEvent);
ZilchDeclareEvent(MouseScroll, MouseEvent);
}//namespace Events

//-----------------------------------------------------------------------------Mouse
class Mouse : public Component
{
public:
  ZilchDeclareType(Mouse, Zilch::TypeCopyMode::ReferenceType);

  Mouse();
  ~Mouse();

  bool IsButtonDown(MouseButtonStates::Enum button);

  void ProcessButton(MouseButtons::Enum button, MouseButtonStates::Enum state);
  void ProcessPosition(const Vec2& position);
  void ProcessMouseScroll(const Vec2& scroll);
  void FilloutEvent(MouseEvent& toSend);
  void ResetMovement();

  MouseButton mButtons[MouseButtons::Count]{};
  Vec2 mPosition;
  Vec2 mMovement;
  Vec2 mScroll;
};

