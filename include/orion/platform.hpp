#pragma once

// OS detection
#if defined(_WIN32)
    #define ORION_PLATFORM_WIN32
    #define ORION_PLATFORM_NAME "Win32"
#elif defined(__linux__)
    #define ORION_PLATFORM_LINUX
    #define ORION_PLATFORM_NAME "Linux"
#else
    #error Unknown/unsupported platform
#endif

// Compiler detection
#if defined(_MSC_VER)
    #define ORION_COMPILER_MSVC
    #define ORION_COMPILER_NAME "MSVC"
#elif defined(__clang__)
    #define ORION_COMPILER_CLANG
    #define ORION_COMPILER_NAME "clang"
#elif defined(__GNUC__)
    #define ORION_COMPILER_GCC
    #define ORION_COMPILER_NAME "gcc"
#endif
