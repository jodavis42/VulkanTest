#include "pch.h"

#include "ShaderEnumTypes.hpp"

#include <unordered_map>

template <typename MapType, typename EnumType>
String GetEnumString(const MapType& map, EnumType enumValue)
{
  auto it = map.find(enumValue);
  if(it != map.end())
    return it->second;
  return "Error";
}

template <typename EnumType>
EnumType GetEnumValue(const std::unordered_map<String, EnumType>& map, const String& enumString)
{
  auto it = map.find(enumString);
  if(it == map.end())
    // Error, not contained
    __debugbreak();
  return it->second;
}

template <typename MapType, typename EnumType>
String GetFlagString(const MapType& map, EnumType enumValue)
{
  String result;
  for(size_t i = 0; i < EnumType::Count; ++i)
  {
    auto mask = enumValue & (1 << i);
    if(mask == 0)
      continue;

    String enumStr = map.find(enumValue)->second;
    if(!result.empty())
      result += "|";
    result += enumStr;
  }
  
  return result;
}

template <typename EnumStruct, typename EnumType>
EnumType GetFlagValue(const std::unordered_map<String, EnumType>& map, const String& enumString)
{
  if(enumString.empty())
    return EnumStruct::All;

  EnumType result = EnumStruct::None;
  size_t startIndex = 0;
  while(startIndex < enumString.size())
  {
    size_t endIndex = enumString.find_first_of('|', startIndex);
    if(endIndex == String::npos)
      endIndex = enumString.size();
    String subStr = enumString.substr(startIndex, endIndex - startIndex);
    result = static_cast<EnumType>(result | map.find(subStr)->second);
    startIndex = endIndex + 1;
  }

  return result;
}

//-------------------------------------------------------------------ShaderPrimitiveType
String ShaderPrimitiveType::ToString(ShaderPrimitiveType::Enum type)
{
  static std::unordered_map<ShaderPrimitiveType::Enum, String> map =
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
  static std::unordered_map<String, ShaderPrimitiveType::Enum> map = 
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
  static std::unordered_map<ShaderResourceType::Enum, String> map =
  {
    {ShaderResourceType::Uniform, "Uniform"},
    {ShaderResourceType::SampledImage, "SampledImage"},
  };
  return GetEnumString(map, type);
}

ShaderResourceType::Enum ShaderResourceType::FromString(const String& typeName)
{
  static std::unordered_map<String, ShaderResourceType::Enum> map =
  {
    {"Uniform", ShaderResourceType::Uniform},
    {"SampledImage", ShaderResourceType::SampledImage},
  };
  return GetEnumValue(map, typeName);
}

//-------------------------------------------------------------------ShaderStage
String ShaderStage::ToString(ShaderStage::Enum type)
{
  static std::unordered_map<ShaderStage::Enum, String> map =
  {
    {ShaderStage::Vertex, "Vertex"},
    {ShaderStage::Pixel, "Pixel"},
  };
  return GetEnumString(map, type);
}

ShaderStage::Enum ShaderStage::FromString(const String& typeName)
{
  static std::unordered_map<String, ShaderStage::Enum> map =
  {
    {"Vertex", ShaderStage::Vertex},
    {"Pixel", ShaderStage::Pixel},
  };
  return GetEnumValue(map, typeName);
}

//-------------------------------------------------------------------ShaderStageFlags
String ShaderStageFlags::ToString(ShaderStageFlags::Enum type)
{
  static std::unordered_map<ShaderStageFlags::Enum, String> map =
  {
    {ShaderStageFlags::Vertex, "Vertex"},
    {ShaderStageFlags::Pixel, "Pixel"},
  };
  return GetFlagString(map, type);
}

ShaderStageFlags::Enum ShaderStageFlags::FromString(const String& typeName)
{
  static std::unordered_map<String, ShaderStageFlags::Enum> map =
  {
    {"Vertex", ShaderStageFlags::Vertex},
    {"Pixel", ShaderStageFlags::Pixel},
  };
  return GetFlagValue<ShaderStageFlags>(map, typeName);
}
