#include "orion/debug.hpp"

#include "orion/log.hpp"
#include "orion/platform.hpp"

#include <cstdio>

namespace orion
{
    void output_debug_string(const char* str)
    {
        if (auto core_logger = Logger::get_core_logger()) {
            core_logger->error(str);
        } else {
            std::fputs(str, stderr);
        }
    }

    void debugbreak()
    {
#if defined(ORION_COMPILER_MSVC)
        __debugbreak();
#elif __has_builtin(__builtin_debugtrap)
        __builtin_debugtrap();
#elif defined(ORION_ARCH_X64) // fallback for x64 gcc
        __asm__ volatile("int3");
#endif
    }

    void unreachable()
    {
#if defined(ORION_COMPILER_MSVC)
        __assume(false);
#else
        __builtin_unreachable();
#endif
    }
} // namespace orion
