#include "orion/platform.h"

#ifdef ORION_PLATFORM_WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
#endif

#define VOLK_IMPLEMENTATION
#include <Volk/volk.h>
