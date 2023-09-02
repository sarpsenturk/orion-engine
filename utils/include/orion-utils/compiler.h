#pragma once

// Compiler detection
#if defined(_MSC_VER)
    #define ORION_COMPILER_MSVC
#elif defined(__clang__)
    #define ORION_COMPILER_CLANG
#elif defined(__GNUC__)
    #define ORION_COMPILER_GCC
#else
    #error "Unsupported compiler. Please check list of supported compilers"
#endif

// Trigger debug break
#if defined(ORION_COMPILER_MSVC)
    #include <intrin.h>
    #define ORION_DEBUG_BREAK() __debugbreak()
#else
    #include <cstdlib>
    #define ORION_DEBUG_BREAK() std::abort()
#endif

// MSVC doesn't support [[no_unique_address]]
#if defined(ORION_COMPILER_MSVC)
    #define ORION_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
    #define ORION_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
