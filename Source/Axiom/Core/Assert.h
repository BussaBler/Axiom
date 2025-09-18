#pragma once
#include "Log.h"

#ifdef AX_ENABLE_ASSERTS
#if defined(_MSC_VER)
#define AX_ASSERT(x, ...) { if (!x) { Axiom::AX_LOG_ERROR("Assertion failed: " #x, __VA_ARGS__); __debugbreak(); } }
#define AX_CORE_ASSERT(x, ...) { if (!x) { Axiom::AX_CORE_LOG_ERROR("Assertion failed: " #x, __VA_ARGS__); __debugbreak(); } }
#elif defined(__GNUC__) || defined(__clang__)
#define AX_ASSERT(x, ...) { if (!x) { Axiom::AX_LOG_ERROR("Assertion failed: " #x, __VA_ARGS__); __builtin_trap(); } }
#define AX_CORE_ASSERT(x, ...) { if (!x) { Axiom::AX_CORE_LOG_ERROR("Assertion failed: " #x, __VA_ARGS__); __builtin_trap(); } }
#endif
#else
#define AX_ASSERT(x, ...) if(!x);
#define AX_CORE_ASSERT(x, ...) if(!x);
#endif // AX_ENABLE_ASSERTS
