///////////////////////////////////////////////////////////////////////////////
/// Authors: Joshua Davis
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "Keyboard.hpp"
#include "Composition.hpp"

namespace Events
{
ZilchDefineEvent(KeyDown);
ZilchDefineEvent(KeyUp);
}//namespace Events

//-----------------------------------------------------------------------------KeyState
bool KeyState::IsDown() const
{
  return mIsDown;
}

bool KeyState::IsUp() const
{
  return !mIsDown;
}

//-----------------------------------------------------------------------------KeyboardEvent
ZilchDefineType(KeyboardEvent, builder, type)
{
  ZilchBindDefaultCopyDestructor();

  ZilchBindGetter(Key);
  ZilchBindGetter(IsDown);
}

KeyboardEvent::KeyboardEvent()
{
  mKey = 0;
  mKeyboard = nullptr;
}

Keys::Enum KeyboardEvent::GetKey() const
{
  return (Keys::Enum)mKey;
}

bool KeyboardEvent::GetIsDown() const
{
  return mKeyState.IsDown();
}

//-----------------------------------------------------------------------------Keyboard
ZilchDefineType(Keyboard, builder, type)
{
  type->HandleManager = ZilchManagerId(Zilch::PointerManager);
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindMethod(IsKeyDown);
  ZilchBindMethod(IsKeyUp);
}

Keyboard::Keyboard()
{
  
}

Keyboard::~Keyboard()
{

}

bool Keyboard::IsKeyDown(Keys::Enum key)
{
  KeyState& keyState = GetKeyState(key);
  return keyState.IsDown();
}

bool Keyboard::IsKeyUp(Keys::Enum key)
{
  KeyState& keyState = GetKeyState(key);
  return keyState.IsUp();
}

KeyState& Keyboard::GetKeyState(Keys::Enum key)
{
  return mKeys[(int)key];
}

void Keyboard::ProcessKey(Keys::Enum key, bool isDown)
{
  if(key < 0 || 255 <= key)
    return;

  KeyState& keyState = mKeys[key];
  if(keyState.mIsDown != isDown)
  {
    keyState.mIsDown = isDown;

    KeyboardEvent toSend;
    toSend.mKeyboard = this;
    toSend.mKey = key;
    toSend.mKeyState = keyState;
    
    toSend.EventName = Events::KeyDown;
    if(!isDown)
      toSend.EventName = Events::KeyUp;

    Zilch::EventSend(GetOwner(), toSend.EventName, &toSend);
  }
}
