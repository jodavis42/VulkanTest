///////////////////////////////////////////////////////////////////////////////
/// Authors: Joshua Davis
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "Mouse.hpp"

#include "Composition.hpp"

namespace Events
{
ZilchDefineEvent(LeftMouseDown);
ZilchDefineEvent(LeftMouseUp);
ZilchDefineEvent(RightMouseDown);
ZilchDefineEvent(RightMouseUp);
ZilchDefineEvent(MiddleMouseDown);
ZilchDefineEvent(MiddleMouseUp);
ZilchDefineEvent(MouseMove);
ZilchDefineEvent(MouseScroll);
}//namespace Events

//-----------------------------------------------------------------------------MouseButton
MouseButton::MouseButton()
{
  mState = MouseButtonStates::Up;
}

//-----------------------------------------------------------------------------MouseEvent
ZilchDefineType(MouseEvent, builder, type)
{
  ZilchBindDefaultCopyDestructor();

  ZilchBindGetter(Button);
  ZilchBindGetter(IsDown);
  ZilchBindField(mPosition);
  ZilchBindField(mMovement);
  ZilchBindField(mScroll);
}

MouseEvent::MouseEvent()
{
  mButton = MouseButtons::None;
  mState = MouseButtonStates::None;
  mMovement = Vec2::cZero;
  mScroll = Vec2::cZero;
  mMouse = nullptr;
}

MouseButtons::Enum MouseEvent::GetButton() const
{
  return mButton;
}

bool MouseEvent::GetIsDown() const
{
  return mState == MouseButtonStates::Down;
}

//-----------------------------------------------------------------------------Mouse
ZilchDefineType(Mouse, builder, type)
{
  type->HandleManager = ZilchManagerId(Zilch::PointerManager);
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindMethod(IsButtonDown);

  ZilchBindFieldGetter(mPosition);
  ZilchBindFieldGetter(mScroll);
  ZilchBindFieldGetter(mMovement);

  builder.AddSendsEvent(type, Events::LeftMouseDown, ZilchTypeId(MouseEvent));
  builder.AddSendsEvent(type, Events::LeftMouseUp, ZilchTypeId(MouseEvent));
  builder.AddSendsEvent(type, Events::RightMouseDown, ZilchTypeId(MouseEvent));
  builder.AddSendsEvent(type, Events::RightMouseUp, ZilchTypeId(MouseEvent));
  builder.AddSendsEvent(type, Events::MiddleMouseDown, ZilchTypeId(MouseEvent));
  builder.AddSendsEvent(type, Events::MiddleMouseUp, ZilchTypeId(MouseEvent));
  builder.AddSendsEvent(type, Events::MouseMove, ZilchTypeId(MouseEvent));
  builder.AddSendsEvent(type, Events::MouseScroll, ZilchTypeId(MouseEvent));
}

Mouse::Mouse()
{
  mPosition = Vec2::cZero;
  mScroll = Vec2::cZero;
  mMovement = Vec2::cZero;
}

Mouse::~Mouse()
{

}

bool Mouse::IsButtonDown(MouseButtonStates::Enum button)
{
  MouseButton& buttonState = mButtons[(int)button];
  return buttonState.mState == MouseButtonStates::Down;
}

void Mouse::ProcessButton(MouseButtons::Enum button, MouseButtonStates::Enum state)
{
  MouseButton& mouseButton = mButtons[button];

  if(mouseButton.mState != state)
  {
    mouseButton.mState = state;

    MouseEvent toSend;
    FilloutEvent(toSend);
    toSend.mButton = button;
    toSend.mState = state;

    if(button == MouseButtons::Left)
    {
      if(state == MouseButtonStates::Down)
        toSend.EventName = Events::LeftMouseDown;
      else if(state == MouseButtonStates::Up)
        toSend.EventName = Events::LeftMouseUp;
    }
    else if(button == MouseButtons::Right)
    {
      if(state == MouseButtonStates::Down)
        toSend.EventName = Events::RightMouseDown;
      else if(state == MouseButtonStates::Up)
        toSend.EventName = Events::RightMouseUp;
    }
    else if(button == MouseButtons::Middle)
    {
      if(state == MouseButtonStates::Down)
        toSend.EventName = Events::MiddleMouseDown;
      else if(state == MouseButtonStates::Up)
        toSend.EventName = Events::MiddleMouseUp;
    }

    Zilch::EventSend(GetOwner(), toSend.EventName, &toSend);
  }
}

void Mouse::ProcessPosition(const Vec2& position)
{
  Vec2 oldPosition = mPosition;
  mPosition = position;
  mMovement = mPosition - oldPosition;

  MouseEvent toSend;
  FilloutEvent(toSend);
  toSend.EventName = Events::MouseMove;
  Zilch::EventSend(GetOwner(), toSend.EventName, &toSend);
}

void Mouse::ProcessMouseScroll(const Vec2& scroll)
{
  mScroll = scroll;

  MouseEvent toSend;
  FilloutEvent(toSend);
  toSend.EventName = Events::MouseScroll;
  Zilch::EventSend(GetOwner(), toSend.EventName, &toSend);
}

void Mouse::FilloutEvent(MouseEvent& toSend)
{
  toSend.mMouse = this;
  toSend.mMovement = mMovement;
  toSend.mPosition = mPosition;
  toSend.mScroll = mScroll;
}

void Mouse::ResetMovement()
{
  mMovement = Vec2::cZero;
}
