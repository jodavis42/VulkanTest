#pragma once

#include <string>
using String = std::string;

#include "Utilities/Asserts.hpp"

class VulkanStatus
{
public:
  static constexpr bool ThrowOnError = true;
  VulkanStatus()
  {
    mFailed = false;
  }

  VulkanStatus(const String& message)
  {
    MarkFailed(message);
  }

  operator bool() const
  {
    return !mFailed;
  }

  void MarkFailed(const String& message)
  {
    mFailed = true;
    mErrorMessage = message;
    if(ThrowOnError)
      ErrorIf(true, mErrorMessage.c_str());
  }

  bool mFailed = false;
  String mErrorMessage;
};
