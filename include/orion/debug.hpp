#pragma once

#include "orion/preprocessor.hpp"

#ifndef ORION_ENABLE_ASSERTIONS
    #ifdef ORION_BUILD_DEBUG
        #define ORION_ENABLE_ASSERTIONS 1
    #else
        #define ORION_ENABLE_ASSERTIONS 0
    #endif
#endif

namespace orion
{
    void output_debug_string(const char* str);
    void debugbreak();
    [[noreturn]] void unreachable();
} // namespace orion

#if ORION_ENABLE_ASSERTIONS
    #define ORION_ASSERT(cond)                                                                                              \
        do {                                                                                                                \
            if (!(cond)) {                                                                                                  \
                ::orion::output_debug_string("Assertion failed (" __FILE__ ":" ORION_XSTR(__LINE__) "): " ORION_STR(cond)); \
                ::orion::debugbreak();                                                                                      \
            }                                                                                                               \
        } while (0)
#else
    #define ORION_ASSERT(cond) \
        do {                   \
        } while (0)
#endif
