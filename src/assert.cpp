#include "orion/assert.hpp"

#include "orion/platform/platform.hpp"
#if defined(ORION_COMPILER_MSVC)
    #define ORION_DEBUG_BREAK() __debugbreak()
#elif defined(ORION_COMPILER_CLANG) || defined(ORION_COMPILER_GCC)
    #if __has_builtin(__builtin_debugtrap) && __has_builtin(__builtin_unreachable)
        #define ORION_DEBUG_BREAK() \
            __builtin_debugtrap();  \
            __builtin_unreachable()
    #else
        #include <signal.h>
        #define ORION_DEBUG_BREAK() raise(SIGTRAP)
    #endif
#endif

#ifdef ORION_OS_WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
#else
    #include <cstdio>
#endif

namespace orion
{
    void debugbreak()
    {
        ORION_DEBUG_BREAK();
    }

    void unreachable()
    {
#if defined(ORION_COMPILER_MSVC)
        __assume(false);
#else
        __builtin_unreachable();
#endif
    }

    void output_debug_string(const char* str)
    {
#ifdef ORION_OS_WIN32
        OutputDebugString(str);
#else
        std::puts(str);
#endif
    }
} // namespace orion
