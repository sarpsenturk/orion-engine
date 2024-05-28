#pragma once

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
#else
    #error "Can't include Windows.h on non-Windows platform"
#endif

#ifndef ORION_WIN32_LOG_LEVEL
    #define ORION_WIN32_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

namespace orion
{
    [[noreturn]] void throw_last_error(DWORD last_error = GetLastError());
} // namespace orion
