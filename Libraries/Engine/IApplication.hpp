#pragma once

#include "EngineStandard.hpp"

#include "Composition.hpp"

class Composition;
class Space;
class GameSession;
class Archetype;

//-------------------------------------------------------------------IApplication
class IApplication
{
public:
  ZilchDeclareType(IApplication, Zilch::TypeCopyMode::ReferenceType);

  virtual Zilch::HandleOf<Composition> CreateComposition(Archetype& archetype) abstract;
  virtual Zilch::HandleOf<Space> CreateSpace(Archetype& archetype) abstract;
  virtual Zilch::HandleOf<GameSession> CreateGame(Archetype& archetype) abstract;
};
