#pragma once

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
