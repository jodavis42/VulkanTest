#pragma once

#include <string>
#include <unordered_map>
#include <vector>
using String = std::string;

struct ShaderStage
{
  enum Enum
  {
    Vertex = 0,
    Pixel,
    Count
  };
};

struct ShaderStageFlags
{
  enum Enum
  {
    None = 0,
    Vertex = 1 << 0,
    Pixel = 1 << 1,
    Mask = (1 << 2) - 1
  };
};

constexpr size_t ShaderStageCount = static_cast<size_t>(ShaderStage::Count);

//-------------------------------------------------------------------ShaderLoadData
struct ShaderLoadData
{
  String mShaderCodePaths[ShaderStageCount]{};
};

//-------------------------------------------------------------------Shader
struct Shader
{
public:
  std::vector<char> mShaderByteCode[ShaderStageCount]{};
};

//-------------------------------------------------------------------ShaderManager
struct ShaderManager
{
public:
  ShaderManager();
  ~ShaderManager();

  void Load();
  void LoadShader(const String& name, const ShaderLoadData& shaderData);
  Shader* Find(const String& name);
  void Destroy();

  std::unordered_map<String, Shader*> mShaderMap;
};
