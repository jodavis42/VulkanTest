#pragma once

#include "GraphicsStandard.hpp"

//-------------------------------------------------------------------ShaderPrimitiveType
struct ShaderPrimitiveType
{
  enum Enum
  {
    Unknown,
    Byte,
    Int,
    Float,
    Float2, Float3, Float4,
    Float2x2, Float3x3, Float4x4,
    SampledImage,
    Struct,
    Count,
    Begin = Unknown,
    End = Count
  };
  static String ToString(ShaderPrimitiveType::Enum type);
  static ShaderPrimitiveType::Enum FromString(const String& typeName);
};

//-------------------------------------------------------------------ShaderResourceType
struct ShaderResourceType
{
  enum Enum
  {
    Unknown = -1,
    Uniform = 0,
    SampledImage,
    Count,
    Begin = Uniform,
    End = Count
  };
  static String ToString(ShaderResourceType::Enum type);
  static ShaderResourceType::Enum FromString(const String& typeName);
};

//-------------------------------------------------------------------ShaderStage
struct ShaderStage
{
  enum Enum
  {
    Vertex = 0,
    Pixel,
    Count,
    Begin = Vertex,
    End = Count
  };
  static String ToString(ShaderStage::Enum type);
  static ShaderStage::Enum FromString(const String& typeName);
};

//-------------------------------------------------------------------ShaderStageFlags
struct ShaderStageFlags
{
  enum Enum
  {
    None = 0,
    Vertex = 1 << 0,
    Pixel = 1 << 1,
    Count = 2,
    End = 1 << 2
  };
  static constexpr Enum All = static_cast<Enum>(End - 1);

  static String ToString(ShaderStageFlags::Enum type);
  static ShaderStageFlags::Enum FromString(const String& typeName);
};
