#pragma once
#include "Log.h"

#ifdef AX_ENABLE_ASSERTS
#define AX_ASSERT(x, ...) { if (!x) { Axiom::AX_LOG_ERROR("Assertion failed: " #x, __VA_ARGS__); __debugbreak(); } }
#define AX_CORE_ASSERT(x, ...) { if (!x) { Axiom::AX_CORE_LOG_ERROR("Assertion failed: " #x, __VA_ARGS__); __debugbreak(); } }
#else 
#define AX_ASSERT(x, ...)
#define AX_CORE_ASSERT(x, ...)
#endif // AX_ENABLE_ASSERTS
