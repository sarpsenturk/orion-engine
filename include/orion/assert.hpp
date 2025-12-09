#pragma once

#define ORION_ASSERT_STRINGIFY2(x) #x
#define ORION_ASSERT_STRINGIFY(x) ORION_ASSERT_STRINGIFY2(x)

#if defined(ORION_ENABLE_ASSERTIONS)
    #define ORION_ASSERT(cond, msg)                                                                                                         \
        do {                                                                                                                                \
            if (!(cond)) {                                                                                                                  \
                ::orion::output_debug_string("Assertion failed (" __FILE__ ":" ORION_ASSERT_STRINGIFY(__LINE__) "): " msg "\n\t"            \
                                                                                                                "Expression: " #cond "\n"); \
                ::orion::debugbreak();                                                                                                      \
            }                                                                                                                               \
        } while (0)
#else
    #define ORION_ASSERT(cond, msg) ((void)0)
#endif

namespace orion
{
    [[noreturn]] void debugbreak();
    void output_debug_string(const char* str);
} // namespace orion
