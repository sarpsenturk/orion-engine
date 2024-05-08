#pragma once

#include <cstdio>
#include <cstdlib>
#include <fmt/format.h>

#if defined(ORION_BUILD_DEBUG) || defined(ORION_ENABLE_ASSERTIONS)
    #define ORION_ASSERT(condition)                                                                                          \
        do {                                                                                                                 \
            if (!(condition)) {                                                                                              \
                std::puts(fmt::format("Assertion failed ({}:{} {}): {}", __FILE__, __LINE__, __func__, #condition).c_str()); \
                std::abort();                                                                                                \
            }                                                                                                                \
        } while (0)
#else
    #define ORION_ASSERT(condition) ((void)0)
#endif

#define ORION_CONDITION_CHECK(type, condition)                                                                       \
    do {                                                                                                             \
        if (!(condition)) [[unlikely]] {                                                                             \
            std::puts(fmt::format(type " failed ({}:{} {}): {}", __FILE__, __LINE__, __func__, #condition).c_str()); \
            std::abort();                                                                                            \
        }                                                                                                            \
    } while (0)

#ifndef ORION_BUILD_DIST
    #define ORION_EXPECTS(condition) ORION_CONDITION_CHECK("Pre-condition", condition)
    #define ORION_ENSURES(condition) ORION_CONDITION_CHECK("Post-condition", condition)
#else
    #define ORION_EXPECTS(condition) ((void)0)
    #define ORION_ENSURES(condition) ((void)0)
#endif

namespace orion
{
#ifdef __cpp_lib_unreachable
    using std::unreachable
#else
    [[noreturn]] inline void unreachable()
    {
    #if defined(_MSC_VER) && !defined(__clang__)
        __assume(false);
    #else
        __builtin_unreachable();
    #endif
    }
#endif
} // namespace orion