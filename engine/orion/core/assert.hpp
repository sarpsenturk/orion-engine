#pragma once

// Assertions can be enabled/disabled by defining ORION_ENABLE_ASSERTIONS to 1 or 0 respectively,
// before including this header file.
// If ORION_ENABLE_ASSERTIONS is not defined, it will be automatically set based on the build configuration:
// - In debug builds (when ORION_DEBUG is defined), assertions will be enabled (ORION_ENABLE_ASSERTIONS = 1).
// - In release builds (when ORION_DEBUG is not defined), assertions will be disabled (ORION_ENABLE_ASSERTIONS = 0).
#ifndef ORION_ENABLE_ASSERTIONS
    #ifdef ORION_DEBUG
        #define ORION_ENABLE_ASSERTIONS 1
    #else
        #define ORION_ENABLE_ASSERTIONS 0
    #endif
#endif

#if ORION_ENABLE_ASSERTIONS
    #define ORION_ASSERT(expression)                                                \
        do {                                                                        \
            if (!(expression)) {                                                    \
                ::orion::invoke_assertion_handler(#expression, __FILE__, __LINE__); \
            }                                                                       \
        } while (0)
#else
    #define ORION_ASSERT(expression) ((void)0)
#endif

namespace orion
{
    void set_assertion_handler(void (*handler)(const char*, const char*, int));
    void invoke_assertion_handler(const char* expression, const char* file, int line);
} // namespace orion
