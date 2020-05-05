#include "Precompiled.hpp"

#include "Material.hpp"

#include <filesystem>
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
    LoadArray(loader, propName, value);
    SetPropertyBuffer(value, data);
    break;
  }
  case ShaderPrimitiveType::Float3:
  {
    Vec3 value;
    LoadArray(loader, propName, value);
    SetPropertyBuffer(value, data);
    break;
  }
  case ShaderPrimitiveType::Float4:
  {
    Vec4 value;
    LoadArray(loader, propName, value);
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

void LoadMaterialProperties(Material* material, JsonLoader& loader)
{
  if(loader.BeginMember("Properties"))
  {
    size_t propCount = 0;
    loader.BeginMembers(propCount);
    material->mProperties.Resize(propCount);
    for(size_t i = 0; i < propCount; ++i)
    {
      MaterialProperty& prop = material->mProperties[i];
      loader.BeginMember(i, prop.mPropertyName);

      prop.mType = ShaderPrimitiveType::FromString(LoadDefaultPrimitive(loader, "PrimitiveType", String()));
      ReadPropertyValue(loader, prop.mType, prop.mData);

      loader.EndMember();
    }
    loader.EndMember();
  }
}

//-------------------------------------------------------------------MaterialManager
MaterialManager::MaterialManager()
{

}

MaterialManager::~MaterialManager()
{
  Destroy();
}

void MaterialManager::Load()
{
  LoadAllFilesOfExtension(*this, "data", ".material");
}

void MaterialManager::LoadFromFile(const String& path)
{
  JsonLoader loader;
  loader.LoadFromFile(path);

  Material* material = new Material();
  material->mMaterialName = LoadDefaultPrimitive(loader, "Name", String());
  material->mShaderName = LoadDefaultPrimitive(loader, "ShaderName", String());
  
  LoadMaterialProperties(material, loader);
  mMaterialMap[material->mMaterialName] = material;
}

void MaterialManager::Add(const String& name, Material* material)
{
  mMaterialMap[name] = material;
}

Material* MaterialManager::Find(const String& name)
{
  return mMaterialMap.FindValue(name, nullptr);
}

void MaterialManager::Destroy()
{
  for(Material* material : mMaterialMap.Values())
    delete material;
  mMaterialMap.Clear();
}
