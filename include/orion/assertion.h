#pragma once

#include "orion/compiler.h"

#ifdef ORION_COMPILER_MSVC
    #define ORION_DEBUG_BREAK() __debugbreak()
#else
    #include <signal.h>
    #define ORION_DEBUG_BREAK() raise(SIGTRAP)
#endif

#define ORION_ASSERT_PRED(pred, type)                                                       \
    do {                                                                                    \
        if (!(pred)) {                                                                      \
            std::fprintf(stderr, #type " failed (%s:%d): %s\n", __FILE__, __LINE__, #pred); \
            ORION_DEBUG_BREAK();                                                            \
        }                                                                                   \
    } while (0)

#ifndef ORION_ENABLE_ASSERTIONS
    #ifdef ORION_BUILD_DEBUG
        #define ORION_ENABLE_ASSERTIONS 1
    #else
        #define ORION_ENABLE_ASSERTIONS 0
    #endif
#endif

#if ORION_ENABLE_ASSERTIONS
    #include <cstdio>
    #define ORION_ASSERT(condition) ORION_ASSERT_PRED(condition, "Assertion")
#else
    #define ORION_ASSERT(condition) ((void)0)
#endif

#ifndef ORION_ENABLE_CONTRACTS
    #ifndef ORION_BUILD_RELEASE
        #define ORION_ENABLE_CONTRACTS 1
    #else
        #define ORION_ENABLE_CONTRACTS 0
    #endif
#endif

#if ORION_ENABLE_CONTRACTS
    #define ORION_EXPECTS(condition) ORION_ASSERT_PRED(condition, "Pre-condition")
    #define ORION_ENSURES(condition) ORION_ASSERT_PRED(condition, "Post-condition")
#else
    #define ORION_EXPECTS(condition) ((void)0)
    #define ORION_ENSURES(condition) ((void)0)
#endif