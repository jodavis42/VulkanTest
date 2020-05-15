#include "Precompiled.hpp"

#include "MaterialShared.hpp"

#include "JsonSerializers.hpp"
#include "Math.hpp"

template <typename T>
void SetPropertyBuffer(const T& data, Array<byte>& buffer)
{
  constexpr size_t size = sizeof(T);
  buffer.Resize(size);
  memcpy(buffer.Data(), &data, size);
}

template <>
void SetPropertyBuffer<String>(const String& str, Array<byte>& buffer)
{
  buffer.Resize(str.SizeInBytes() + 1);
  memcpy(buffer.Data(), str.Data(), str.SizeInBytes());
  buffer[str.SizeInBytes()] = '\0';
}

void ReadPropertyValue(JsonLoader& loader, ShaderPrimitiveType::Enum& propType, Array<byte>& data)
{
  String propName = "Value";
  switch(propType)
  {
  case ShaderPrimitiveType::Byte:
  {
    char value = LoadDefaultPrimitive<char>(loader, propName, 0);
    SetPropertyBuffer(value, data);
    break;
  }
  case ShaderPrimitiveType::Int:
  {
    int value = LoadDefaultPrimitive<int>(loader, propName, 0);
    SetPropertyBuffer(value, data);
    break;
  }
  case ShaderPrimitiveType::Float:
  {
    float value = LoadDefaultPrimitive<float>(loader, propName, 0.0f);
    SetPropertyBuffer(value, data);
    break;
  }
  case ShaderPrimitiveType::Float2:
  {
    Vec2 value;
    LoadArray<Vec2, 2>(loader, propName, value);
    SetPropertyBuffer(value, data);
    break;
  }
  case ShaderPrimitiveType::Float3:
  {
    Vec3 value;
    LoadArray<Vec3, 3>(loader, propName, value);
    SetPropertyBuffer(value, data);
    break;
  }
  case ShaderPrimitiveType::Float4:
  {
    Vec4 value;
    LoadArray<Vec4, 4>(loader, propName, value);
    SetPropertyBuffer(value, data);
    break;
  }
  case ShaderPrimitiveType::SampledImage:
  {
    String value = LoadDefaultPrimitive(loader, propName, String());
    SetPropertyBuffer(value, data);
    break;
  }
  }
}

void LoadMaterialProperty(JsonLoader& loader, const String& propertyName, MaterialProperty& materialProperty)
{
  materialProperty.mPropertyName = propertyName;

  materialProperty.mType = ShaderPrimitiveType::FromString(LoadDefaultPrimitive(loader, "PrimitiveType", String()));
  ReadPropertyValue(loader, materialProperty.mType, materialProperty.mData);
}

void LoadMaterialProperties(JsonLoader& loader, Array<MaterialProperty>& materialProperties)
{
  if(loader.BeginMember("Properties"))
  {
    size_t propCount = 0;
    loader.BeginMembers(propCount);
    materialProperties.Resize(propCount);
    for(size_t i = 0; i < propCount; ++i)
    {
      MaterialProperty& prop = materialProperties[i];
      loader.BeginMember(i, prop.mPropertyName);
      LoadMaterialProperty(loader, prop.mPropertyName, prop);
      loader.EndMember();
    }
    loader.EndMember();
  }
}
