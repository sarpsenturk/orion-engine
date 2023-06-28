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

#include <spdlog/logger.h>                   // spdlog::logger
#include <spdlog/sinks/stdout_color_sinks.h> // spdlog::stdout_color_mt

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
            (LPSTR)&buffer,
            0,
            nullptr);
        // TODO: Check if result is 0 recursively?

        // We have to do this dance since the buffer allocated with
        // LocalAlloc has to be freed with LocalFree
        try {
            std::string message(buffer, result);
            LocalFree(buffer);
            return message;
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

        [[nodiscard]] const char* type() const noexcept override { return "Win32Error"; }
        [[nodiscard]] int return_code() const noexcept override { return error_code_; }

        DWORD error_code_;
    };

    namespace win32
    {
        inline spdlog::logger* logger()
        {
            static const auto win32_logger = []() {
                auto logger = spdlog::stdout_color_mt("orion-win32");
                logger->set_pattern("[%n] [%^%l%$] %v");
                logger->set_level(static_cast<spdlog::level::level_enum>(ORION_WIN32_LOG_LEVEL));
                return logger;
            }();
            return win32_logger.get();
        }
    } // namespace win32
} // namespace orion
