#include "Precompiled.hpp"

#include "CompositionInitializer.hpp"

//-------------------------------------------------------------------CompositionCreationContext
ZilchDefineType(CompositionCreationContext, builder, type)
{
  type->CreatableInScript = true;
  ZilchBindDefaultCopyDestructor();

  ZilchBindFieldProperty(mScale);
  ZilchBindFieldProperty(mRotation);
  ZilchBindFieldProperty(mTranslation);
}
