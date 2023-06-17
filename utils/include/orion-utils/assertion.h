#pragma once

#include <cstdio>       // std::puts
#include <cstdlib>      // std::abort
#include <fmt/format.h> // fmt::format

#if !defined(NDEBUG) || !defined(ORION_ENABLE_ASSERTIONS)
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

#define ORION_EXPECTS(condition) ORION_CONDITION_CHECK("Pre-condition", condition)
#define ORION_ENSURES(condition) ORION_CONDITION_CHECK("Post-condition", condition)
