#pragma once

#include "globals.h"

namespace VorGen {

#if defined __GNUG__
    #define OFFSETOF __builtin_offsetof
    #define ALIGN(X) __attribute__ ((aligned (X)))
#elif defined _MSC_VER
    #define OFFSETOF offsetof
    #define ALIGN(X) __declspec(align(X))
#else
    #error 'unsupported compiler'
#endif

}