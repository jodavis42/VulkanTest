#pragma once

#include "Common/CommonStandard.hpp"
#undef Error
using Zero::Array;
using Zero::String;

inline static Array<char> readFile(const String& filename)
{
  Zero::DataBlock block = Zero::ReadFileIntoDataBlock(filename.c_str());
  Array<char> result;
  result.Resize(block.Size);
  memcpy(result.Data(), block.Data, block.Size);
  return result;
}

inline static void readFile(const String& filename, Array<char>& data)
{
  Zero::DataBlock block = Zero::ReadFileIntoDataBlock(filename.c_str());
  data.Resize(block.Size);
  memcpy(data.Data(), block.Data, block.Size);
}
