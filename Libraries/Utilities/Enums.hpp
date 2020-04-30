#pragma once

#define DeclareEnumOperator(EnumType, Op)                                                        \
EnumType operator##Op(EnumType lhs, EnumType rhs)                                                \
{                                                                                                \
  return static_cast<EnumType>(static_cast<unsigned int>(lhs) Op static_cast<unsigned int>(rhs));\
}

#define DeclareEnumOperators(EnumType) \
DeclareEnumOperator(EnumType, &)       \
DeclareEnumOperator(EnumType, |)
