#include "orion-core/platform/win32/win32_dl_lib.h"

#include <spdlog/spdlog.h> // SPDLOG_*

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
                SPDLOG_ERROR("LoadLibrary failed! LastError: {}", last_error);
                throw Win32Error(last_error);
            }
            SPDLOG_TRACE("LoadLibrary(\"{}\") successful (HMODULE: {})", filename, fmt::ptr(hmodule));
            return new PlatformModule(hmodule);
        }

        void free_library(PlatformModule* platform_module)
        {
            if (platform_module) {
                HMODULE hmodule = platform_module->hmodule();
                FreeLibrary(hmodule);
                SPDLOG_TRACE("Freed HMODULE {}", fmt::ptr(hmodule));
            }
        }

        void* load_library_address(PlatformModule* platform_module, const char* symbol)
        {
            if (!platform_module) {
                SPDLOG_ERROR("Calling load_library_address with platform_module == nullptr!");
                return nullptr;
            }
            HMODULE hmodule = platform_module->hmodule();
            if (FARPROC addr = GetProcAddress(hmodule, symbol)) {
                return reinterpret_cast<void*>(addr);
            }
            const auto last_error = GetLastError();
            SPDLOG_ERROR("GetProcAddress({}, {}) failed! LastError: {}", fmt::ptr(hmodule), symbol, last_error);
            throw Win32Error(last_error);
        }
    } // namespace platform
} // namespace orion