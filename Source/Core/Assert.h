#pragma once
#include "Log.h"

#ifdef AX_ENABLE_ASSERTS
#if defined(_MSC_VER)
#define AX_ASSERT(x, ...)                                                                                                                                      \
    do {                                                                                                                                                       \
        if (!(x)) {                                                                                                                                            \
            Axiom::AX_LOG_ERROR("Assertion failed: " #x " - " __VA_ARGS__);                                                                                    \
            __debugbreak();                                                                                                                                    \
        }                                                                                                                                                      \
    } while (0)
#define AX_CORE_ASSERT(x, ...)                                                                                                                                 \
    do {                                                                                                                                                       \
        if (!(x)) {                                                                                                                                            \
            Axiom::AX_CORE_LOG_ERROR("Assertion failed: " #x " - " __VA_ARGS__);                                                                               \
            __debugbreak();                                                                                                                                    \
        }                                                                                                                                                      \
    } while (0)
#elif defined(__GNUC__) || defined(__clang__)
#define AX_ASSERT(x, ...)                                                                                                                                      \
    do {                                                                                                                                                       \
        if (!(x)) {                                                                                                                                            \
            Axiom::AX_LOG_ERROR("Assertion failed: " #x " - " __VA_ARGS__);                                                                                    \
            __builtin_trap();                                                                                                                                  \
        }                                                                                                                                                      \
    } while (0)
#define AX_CORE_ASSERT(x, ...)                                                                                                                                 \
    do {                                                                                                                                                       \
        if (!(x)) {                                                                                                                                            \
            Axiom::AX_CORE_LOG_ERROR("Assertion failed: " #x " - " __VA_ARGS__);                                                                               \
            __builtin_trap();                                                                                                                                  \
        }                                                                                                                                                      \
    } while (0)
#endif
#else
#define AX_ASSERT(x, ...) (void)0
#define AX_CORE_ASSERT(x, ...) (void)0
#endif // AX_ENABLE_ASSERTS
