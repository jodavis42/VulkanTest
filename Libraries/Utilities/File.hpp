#pragma once

#include <vector>
#include <fstream>

inline static std::vector<char> readFile(const std::string& filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if(!file.is_open())
    throw std::runtime_error("failed to open file!");

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

inline static void readFile(const std::string& filename, std::vector<char>& data)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if(!file.is_open())
    throw std::runtime_error("failed to open file!");

  size_t fileSize = (size_t)file.tellg();
  data.resize(fileSize);
  file.seekg(0);
  file.read(data.data(), fileSize);
  file.close();
}
