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
#ifdef ORION_COMPILER_MSVC
        __debugbreak();
#else
        __builtin_debugtrap();
#endif
    }
} // namespace orion
