#include "orion-core/platform/win32/win32_dl_lib.h"

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
                throw Win32Error();
            }
            return new PlatformModule(hmodule);
        }

        void free_library(PlatformModule* platform_module)
        {
            if (platform_module) {
                FreeLibrary(platform_module->hmodule());
            }
        }

        void* load_library_address(PlatformModule* platform_module, const char* symbol)
        {
            if (!platform_module) {
                return nullptr;
            }
            if (FARPROC addr = GetProcAddress(platform_module->hmodule(), symbol)) {
                return reinterpret_cast<void*>(addr);
            }
            throw Win32Error();
        }
    } // namespace platform
} // namespace orion
