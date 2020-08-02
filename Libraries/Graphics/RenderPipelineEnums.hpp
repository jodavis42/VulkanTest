#pragma once

//-------------------------------------------------------------------BlendFactor
namespace BlendFactor
{
enum Enum
{
  Zero,
  One,
  SourceColor,
  InvSourceColor,
  DestColor,
  InvDestColor,
  SourceAlpha,
  InvSourceAlpha,
  DestAlpha,
  InvDestAlpha,
  SourceAlphaSaturage,
};
}

//-------------------------------------------------------------------BlendEquation
namespace BlendEquation
{
enum Enum
{
  Add,
  Subtract,
  ReverseSubtract,
  Min,
  Max
};
}

//-------------------------------------------------------------------BlendMode
namespace BlendMode
{
enum Enum
{
  Disabled,
  Enabled
};
}

//-------------------------------------------------------------------DepthMode
namespace DepthMode
{
enum Enum
{
  Disabled,
  Read,
  Write,
  ReadWrite
};
}

//-------------------------------------------------------------------TextureCompareFunc
namespace TextureCompareFunc
{
enum Enum
{
  Never, Always, Less, LessEqual, Greater, GreaterEqual, Equal, NotEqual
};
}
