#include "Precompiled.hpp"

#include "ShaderEnumTypes.hpp"

template <typename MapType, typename EnumType>
String GetEnumString(const MapType& map, EnumType enumValue)
{
  return map.FindValue(enumValue, "Error");
}

template <typename EnumType>
EnumType GetEnumValue(const HashMap<String, EnumType>& map, const String& enumString)
{
  EnumType* result = map.FindPointer(enumString);
  if(result == nullptr)
    // Error, not contained
    __debugbreak();
  return *result;
}

template <typename MapType, typename EnumType>
String GetFlagString(const MapType& map, EnumType enumValue)
{
  Zero::StringBuilder result;
  for(size_t i = 0; i < EnumType::Count; ++i)
  {
    auto mask = enumValue & (1 << i);
    if(mask == 0)
      continue;

    String enumStr = map.FindValue(enumValue, String());
    if(!result.GetSize() == 0)
      result.Append("|");
    result.Append(enumStr);
  }
  
  return result.ToString();
}

template <typename EnumStruct, typename EnumType>
EnumType GetFlagValue(const HashMap<String, EnumType>& map, const String& enumString)
{
  if(enumString.Empty())
    return EnumStruct::All;

  EnumType result = EnumStruct::None;
  Zero::StringSplitRange range = enumString.Split("|");
  while(!range.Empty())
  {
    String subStr = range.Front();
    auto enumValuePtr = map.FindPointer(subStr);
    result = static_cast<EnumType>(result | *enumValuePtr);
    range.PopFront();
  }

  return result;
}

//-------------------------------------------------------------------ShaderPrimitiveType
String ShaderPrimitiveType::ToString(ShaderPrimitiveType::Enum type)
{
  static HashMap<ShaderPrimitiveType::Enum, String> map =
  {
    {ShaderPrimitiveType::Unknown, "Unknown"},
    {ShaderPrimitiveType::Byte, "Byte"},
    {ShaderPrimitiveType::Int, "Int"},
    {ShaderPrimitiveType::Float, "Float"},
    {ShaderPrimitiveType::Float2, "Float2"},
    {ShaderPrimitiveType::Float3, "Float3"},
    {ShaderPrimitiveType::Float4, "Float4"},
    {ShaderPrimitiveType::Float2x2, "Float2x2"},
    {ShaderPrimitiveType::Float3x3, "Float3x3"},
    {ShaderPrimitiveType::Float4x4, "Float4x4"},
    {ShaderPrimitiveType::SampledImage, "SampledImage"},
    {ShaderPrimitiveType::Struct, "Struct"}
  };
  return GetEnumString(map, type);
}

ShaderPrimitiveType::Enum ShaderPrimitiveType::FromString(const String& typeName)
{
  static HashMap<String, ShaderPrimitiveType::Enum> map = 
  {
    {"Unknown", ShaderPrimitiveType::Unknown},
    {"Byte", ShaderPrimitiveType::Byte},
    {"Int", ShaderPrimitiveType::Int},
    {"Float", ShaderPrimitiveType::Float},
    {"Float2", ShaderPrimitiveType::Float2},
    {"Float3", ShaderPrimitiveType::Float3},
    {"Float4", ShaderPrimitiveType::Float4},
    {"Float2x2", ShaderPrimitiveType::Float2x2},
    {"Float3x3", ShaderPrimitiveType::Float3x3},
    {"Float4x4", ShaderPrimitiveType::Float4x4},
    {"SampledImage", ShaderPrimitiveType::SampledImage},
    {"Struct", ShaderPrimitiveType::Struct}
  };
  return GetEnumValue(map, typeName);
}

//-------------------------------------------------------------------ShaderResourceType
String ShaderResourceType::ToString(ShaderResourceType::Enum type)
{
  static HashMap<ShaderResourceType::Enum, String> map =
  {
    {ShaderResourceType::Uniform, "Uniform"},
    {ShaderResourceType::SampledImage, "SampledImage"},
  };
  return GetEnumString(map, type);
}

ShaderResourceType::Enum ShaderResourceType::FromString(const String& typeName)
{
  static HashMap<String, ShaderResourceType::Enum> map =
  {
    {"Uniform", ShaderResourceType::Uniform},
    {"SampledImage", ShaderResourceType::SampledImage},
  };
  return GetEnumValue(map, typeName);
}

//-------------------------------------------------------------------ShaderStage
String ShaderStage::ToString(ShaderStage::Enum type)
{
  static HashMap<ShaderStage::Enum, String> map =
  {
    {ShaderStage::Vertex, "Vertex"},
    {ShaderStage::Pixel, "Pixel"},
  };
  return GetEnumString(map, type);
}

ShaderStage::Enum ShaderStage::FromString(const String& typeName)
{
  static HashMap<String, ShaderStage::Enum> map =
  {
    {"Vertex", ShaderStage::Vertex},
    {"Pixel", ShaderStage::Pixel},
  };
  return GetEnumValue(map, typeName);
}

//-------------------------------------------------------------------ShaderStageFlags
String ShaderStageFlags::ToString(ShaderStageFlags::Enum type)
{
  static HashMap<ShaderStageFlags::Enum, String> map =
  {
    {ShaderStageFlags::Vertex, "Vertex"},
    {ShaderStageFlags::Pixel, "Pixel"},
  };
  return GetFlagString(map, type);
}

ShaderStageFlags::Enum ShaderStageFlags::FromString(const String& typeName)
{
  static HashMap<String, ShaderStageFlags::Enum> map =
  {
    {"Vertex", ShaderStageFlags::Vertex},
    {"Pixel", ShaderStageFlags::Pixel},
  };
  return GetFlagValue<ShaderStageFlags>(map, typeName);
}

ShaderStageFlags::Enum operator|(ShaderStageFlags::Enum lhs, ShaderStageFlags::Enum rhs)
{
  return static_cast<ShaderStageFlags::Enum>((int)lhs | (int)rhs);
}

ShaderStageFlags::Enum ShaderStageEnumToFlags(ShaderStage::Enum enumVal)
{
  return (ShaderStageFlags::Enum)(1 << enumVal);
}
