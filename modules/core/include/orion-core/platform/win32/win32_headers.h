#pragma once

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
#else
    #error "Can't include Windows.h if _WIN32 isn't defined"
#endif

#include "orion-core/exception.h"

namespace orion
{
    inline std::string win32_format_last_error(DWORD last_error)
    {
        char* buffer = nullptr;
        auto result = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            last_error,
            0,
            buffer,
            0,
            nullptr);
        // TODO: Check if result is 0 recursively?

        // We have to do this dance since the buffer allocated with
        // LocalAlloc has to be freed with LocalFree
        try {
            std::string message(buffer, result);
            LocalFree(buffer);
            return buffer;
        } catch (...) {
            LocalFree(buffer);
            throw;
        }
    }

    class Win32Error : public OrionException
    {
    public:
        explicit Win32Error(DWORD last_error = GetLastError())
            : OrionException(win32_format_last_error(last_error))
            , error_code_(last_error)
        {
        }

        const char* type() const noexcept override { return "Win32Error"; }
        int return_code() const noexcept override { return error_code_; }

        DWORD error_code_;
    };
} // namespace orion
