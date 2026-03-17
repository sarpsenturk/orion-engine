#pragma once

#if defined(_WIN32)
    #define ORION_PLATFORM_WINDOWS 1
    #define ORION_PLATFORM_NAME "Windows"
#elif defined(__linux__)
    #define ORION_PLATFORM_LINUX 1
    #define ORION_PLATFORM_NAME "Linux"
#elif defined(__apple__)
    #define ORION_PLATFORM_MACOS 1
    #define ORION_PLATFORM_NAME "macOS"
#else
    #error "Unknown/unsupported platform"
#endif

#if defined(__clang__)
    #define ORION_COMPILER_CLANG 1
    #define ORION_COMPILER_NAME "clang"
#elif defined(__GNUC__)
    #define ORION_COMPILER_GCC 1
    #define ORION_COMPILER_NAME "gcc"
#elif defined(_MSC_VER)
    #define ORION_COMPILER_MSVC 1
    #define ORION_COMPILER_NAME "MSVC"
#else
    #error "Unknown/unsupported compiler"
#endif

#if defined(__x86_64__) || defined(_M_X64)
    #define ORION_ARCH_X64 1
    #define ORION_ARCH_NAME "x64"
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ORION_ARCH_ARM64 1
    #define ORION_ARCH_NAME "ARM64"
#else
    #error "Uknown/unsupported cpu architecture"
#endif
