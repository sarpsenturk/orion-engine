#include "orion-win32/win32_dyn_lib.h"
#include "orion-platform/platform.h"

#include <spdlog/spdlog.h> // SPDLOG_LOGGER_*

namespace orion
{
    PlatformModule::PlatformModule(HMODULE hmodule)
        : hmodule_(hmodule)
    {
    }

    namespace platform
    {
        PlatformModule* load_library(const char* filename)
        {
            HMODULE hmodule = LoadLibrary(filename);
            if (!hmodule) {
                const auto last_error = GetLastError();
                SPDLOG_LOGGER_ERROR(platform_logger(), "Failed to load dynamic library '{}'! LastError: {}", filename, last_error);
                throw_last_error(last_error);
            }
            SPDLOG_LOGGER_TRACE(platform_logger(), "LoadLibrary(\"{}\") successful (HMODULE: {})", filename, fmt::ptr(hmodule));
            return new PlatformModule(hmodule);
        }

        void free_library(PlatformModule* platform_module)
        {
            if (platform_module) {
                HMODULE hmodule = platform_module->hmodule();
                FreeLibrary(hmodule);
                delete platform_module;
                SPDLOG_LOGGER_TRACE(platform_logger(), "Freed HMODULE {}", fmt::ptr(hmodule));
            }
        }

        void* load_library_address(PlatformModule* platform_module, const char* symbol)
        {
            if (!platform_module) {
                SPDLOG_LOGGER_ERROR(platform_logger(), "Calling load_library_address with platform_module == nullptr!");
                return nullptr;
            }
            HMODULE hmodule = platform_module->hmodule();
            if (FARPROC addr = GetProcAddress(hmodule, symbol)) {
                return reinterpret_cast<void*>(addr);
            }
            const auto last_error = GetLastError();
            SPDLOG_LOGGER_ERROR(platform_logger(), "GetProcAddress({}, {}) failed! LastError: {}", fmt::ptr(hmodule), symbol, last_error);
            return nullptr;
        }
    } // namespace platform
} // namespace orion
