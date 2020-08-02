#include "Precompiled.hpp"

#include "GraphicsZilchStaticLibrary.hpp"

#include "Engine/EngineZilchStaticLibrary.hpp"

#include "Camera.hpp"
#include "Mesh.hpp"
#include "Model.hpp"
#include "GraphicsSpace.hpp"
#include "GraphicsEngine.hpp"
#include "NativeRenderer.hpp"
#include "RenderGroup.hpp"
#include "RenderTasks.hpp"
#include "Texture.hpp"
#include "ZilchMaterial.hpp"
#include "ZilchFragment.hpp"

ZilchDefineExternalBaseType(BlendFactor::Enum, Zilch::TypeCopyMode::ValueType, builder, type)
{
  ZilchFullBindEnum(builder, type, Zilch::SpecialType::Enumeration);

  ZilchFullBindEnumValue(builder, type, BlendFactor::Zero, "Zero");
  ZilchFullBindEnumValue(builder, type, BlendFactor::One, "One");
  ZilchFullBindEnumValue(builder, type, BlendFactor::SourceColor, "SourceColor");
  ZilchFullBindEnumValue(builder, type, BlendFactor::InvSourceColor, "InvSourceColor");
  ZilchFullBindEnumValue(builder, type, BlendFactor::DestColor, "DestColor");
  ZilchFullBindEnumValue(builder, type, BlendFactor::InvDestColor, "InvDestColor");
  ZilchFullBindEnumValue(builder, type, BlendFactor::SourceAlpha, "SourceAlpha");
  ZilchFullBindEnumValue(builder, type, BlendFactor::InvSourceAlpha, "InvSourceAlpha");
  ZilchFullBindEnumValue(builder, type, BlendFactor::DestAlpha, "DestAlpha");
  ZilchFullBindEnumValue(builder, type, BlendFactor::InvDestAlpha, "InvDestAlpha");
  ZilchFullBindEnumValue(builder, type, BlendFactor::SourceAlphaSaturage, "SourceAlphaSaturage");
}

ZilchDefineExternalBaseType(BlendEquation::Enum, Zilch::TypeCopyMode::ValueType, builder, type)
{
  ZilchFullBindEnum(builder, type, Zilch::SpecialType::Enumeration);

  ZilchFullBindEnumValue(builder, type, BlendEquation::Add, "Add");
  ZilchFullBindEnumValue(builder, type, BlendEquation::Subtract, "Subtract");
  ZilchFullBindEnumValue(builder, type, BlendEquation::ReverseSubtract, "ReverseSubtract");
  ZilchFullBindEnumValue(builder, type, BlendEquation::Min, "Min");
  ZilchFullBindEnumValue(builder, type, BlendEquation::Max, "Max");
}

ZilchDefineExternalBaseType(BlendMode::Enum, Zilch::TypeCopyMode::ValueType, builder, type)
{
  ZilchFullBindEnum(builder, type, Zilch::SpecialType::Enumeration);

  ZilchFullBindEnumValue(builder, type, BlendMode::Disabled, "Disabled");
  ZilchFullBindEnumValue(builder, type, BlendMode::Enabled, "Enabled");
}

ZilchDefineExternalBaseType(DepthMode::Enum, Zilch::TypeCopyMode::ValueType, builder, type)
{
  ZilchFullBindEnum(builder, type, Zilch::SpecialType::Enumeration);

  ZilchFullBindEnumValue(builder, type, DepthMode::Disabled, "Disabled");
  ZilchFullBindEnumValue(builder, type, DepthMode::Read, "Read");
  ZilchFullBindEnumValue(builder, type, DepthMode::Write, "Write");
  ZilchFullBindEnumValue(builder, type, DepthMode::ReadWrite, "ReadWrite");
}

ZilchDefineExternalBaseType(TextureCompareFunc::Enum, Zilch::TypeCopyMode::ValueType, builder, type)
{
  ZilchFullBindEnum(builder, type, Zilch::SpecialType::Enumeration);

  ZilchFullBindEnumValue(builder, type, TextureCompareFunc::Never, "Never");
  ZilchFullBindEnumValue(builder, type, TextureCompareFunc::Always, "Always");
  ZilchFullBindEnumValue(builder, type, TextureCompareFunc::Less, "Less");
  ZilchFullBindEnumValue(builder, type, TextureCompareFunc::LessEqual, "LessEqual");
  ZilchFullBindEnumValue(builder, type, TextureCompareFunc::Greater, "Greater");
  ZilchFullBindEnumValue(builder, type, TextureCompareFunc::GreaterEqual, "GreaterEqual");
  ZilchFullBindEnumValue(builder, type, TextureCompareFunc::Equal, "Equal");
  ZilchFullBindEnumValue(builder, type, TextureCompareFunc::NotEqual, "NotEqual");
}

ZilchDefineStaticLibrary(GraphicsStaticLibrary)
{
  builder.CreatableInScriptDefault = false;

   // Enums
  ZilchInitializeExternalTypeAs(BlendFactor::Enum, "BlendFactor");
  ZilchInitializeExternalTypeAs(BlendEquation::Enum, "BlendEquation");
  ZilchInitializeExternalTypeAs(BlendMode::Enum, "BlendMode");
  ZilchInitializeExternalTypeAs(DepthMode::Enum, "DepthMode");
  ZilchInitializeExternalTypeAs(TextureCompareFunc::Enum, "TextureCompareFunc");

  ZilchInitializeType(Mesh);
  ZilchInitializeType(MeshManager);
  ZilchInitializeType(ZilchMaterial);
  ZilchInitializeType(ZilchMaterialManager);
  ZilchInitializeType(ZilchFragmentFile);
  ZilchInitializeType(ZilchFragmentFileManager);
  ZilchInitializeType(Texture);
  ZilchInitializeType(TextureManager);
  ZilchInitializeType(RenderGroup);
  ZilchInitializeType(RenderGroupManager);

  ZilchInitializeType(RenderGroupSet);
  ZilchInitializeType(BlendSettings);
  ZilchInitializeType(DepthSettings);
  ZilchInitializeType(RenderPipelineSettings);
  ZilchInitializeType(RenderTask);
  ZilchInitializeType(ClearTargetRenderTask);
  ZilchInitializeType(RenderGroupRenderTask);
  ZilchInitializeType(RenderTaskEvent);

  ZilchInitializeType(Graphical);
  ZilchInitializeType(Camera);
  ZilchInitializeType(Model);
  ZilchInitializeType(NativeRenderer);
  ZilchInitializeType(GraphicsSpace);
  ZilchInitializeType(GraphicsEngine);

  AddNativeLibraryExtensions(builder);
}
