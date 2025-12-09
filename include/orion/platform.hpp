#pragma once

// OS detection
#if defined(_WIN32)
    #define ORION_OS_WIN32 1
    #define ORION_OS_NAME "Windows"
#elif defined(__linux__)
    #define ORION_OS_LINUX 1
    #define ORION_OS_NAME "Linux"
#elif defined(__APPLE__)
    #define ORION_OS_MACOS 1
    #define ORION_OS_NAME "macOS"
#else
    #define ORION_OS_UNKNOWN 1
    #define ORION_OS_NAME "unknown"
#endif

// Architecture detection
#if defined(__x86_64__) || defined(_M_X64)
    #define ORION_ARCH_X86_64 1
    #define ORION_ARCH_NAME "x86_64"
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ORION_ARCH_ARM64 1
    #define ORION_ARCH_NAME "ARM64";
#else
    #define ORION_ARCH_UNKNOWN 1
    #define ORION_ARCH_NAME "unknown"
#endif

// Compiler detection
#if defined(_MSC_VER)
    #define ORION_COMPILER_MSVC 1
    #define ORION_COMPILER_NAME "MSVC"
#elif defined(__clang__)
    #define ORION_COMPILER_CLANG 1
    #define ORION_COMPILER_NAME "Clang"
#elif defined(__GNUC__)
    #define ORION_COMPILER_GCC 1
    #define ORION_COMPILER_NAME "gcc"
#else
    #define ORION_COMPILER_UNKNOWN 1
    #define ORION_COMPILER_NAME "unknown"
#endif
