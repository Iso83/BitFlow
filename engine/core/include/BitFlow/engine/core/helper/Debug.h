#pragma once

#include <cassert>

#ifdef _MSC_VER
#define BF_CORE_ASSERT(expr) _ASSERT(expr)
#else
#define BF_CORE_ASSERT(expr) assert(expr)
#endif