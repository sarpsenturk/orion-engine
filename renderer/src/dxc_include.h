#pragma once

#include "orion-core/config.h"

#if ORION_PLATFORM_WINDOWS
    #include <combaseapi.h> // IID_PPV_ARGS
    #include <wrl/client.h> // ComPtr
template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
#else
    #error "DXC support has not been implemented for this platform"
#endif

#include <dxcapi.h>
