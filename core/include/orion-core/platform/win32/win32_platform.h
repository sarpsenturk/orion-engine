#pragma once

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
#else
    #error "Can't include Windows.h on non-Windows platform"
#endif

#include "orion-core/exception.h"

#ifndef ORION_WIN32_LOG_LEVEL
    #define ORION_WIN32_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#include "orion-core/log.h"

namespace orion
{
    std::string win32_format_last_error(DWORD last_error);

    class Win32Error : public OrionException
    {
    public:
        explicit Win32Error(DWORD last_error = GetLastError())
            : OrionException(win32_format_last_error(last_error))
            , error_code_(last_error)
        {
        }

        [[nodiscard]] const char* type() const noexcept override { return "Win32Error"; }
        [[nodiscard]] int return_code() const noexcept override { return error_code_; }

        DWORD error_code_;
    };

    namespace win32
    {
        spdlog::logger* logger();
    }
} // namespace orion
