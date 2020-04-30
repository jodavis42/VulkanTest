#pragma once

#define ErrorIf(expression, message) \
do                                   \
{                                    \
  if(expression)                     \
    __debugbreak();                  \
} while(false)                       
