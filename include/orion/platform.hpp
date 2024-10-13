#pragma once

#if defined(_WIN32)
    #define ORION_PLATFORM_WIN32
    #define ORION_PLATFORM_NAME "Win32"
#elif defined(__linux__)
    #define ORION_PLATFORM_LINUX
    #define ORION_PLATFORM_NAME "Linux"
#else
    #error Unknown/unsupported platform
#endif
