#pragma once

#include "Zilch/Zilch.hpp"

struct ApplicationConfig
{
  ApplicationConfig();
  ~ApplicationConfig();

  Zilch::ZilchSetup* mZilchSetup = nullptr;
  Zilch::Module* mNativeModule = nullptr;
};
