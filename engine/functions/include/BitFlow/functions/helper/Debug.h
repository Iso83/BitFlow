#pragma once

#include <cassert>

#ifdef _MSC_VER
#define BF_FUNCTIONS_ASSERT(expr) _ASSERT(expr)
#else
#define BF_FUNCTION_ASSERT(expr) assert(expr)
#endif