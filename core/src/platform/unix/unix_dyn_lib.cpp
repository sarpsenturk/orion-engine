#include "orion-core/platform/unix/unix_dny_lib.h"

#include "orion-core/platform/unix/unix_platform.h"

#include <spdlog/spdlog.h>

#include <dlfcn.h>

namespace orion
{
    PlatformModule::PlatformModule(void* handle)
        : handle_(handle)
    {
    }

    namespace platform
    {
        PlatformModule* load_library(const char* filename)
        {
            void* handle = dlopen(filename, 0);
            if (handle == nullptr) {
                SPDLOG_LOGGER_ERROR(unix::logger(), "{}", dlerror());
                return nullptr;
            }
            return new PlatformModule{handle};
        }

        void free_library(PlatformModule* platform_module)
        {
            if (platform_module == nullptr) {
                return;
            }
            if (dlclose(platform_module->handle()) == -1) {
                SPDLOG_LOGGER_ERROR(unix::logger(), "{}", dlerror());
            }
        }

        void* load_library_address(PlatformModule* platform_module, const char* symbol)
        {
            void* address = dlsym(platform_module->handle(), symbol);
            if (address == nullptr) {
                SPDLOG_LOGGER_ERROR(unix::logger(), "{}", dlerror());
            }
            return address;
        }
    } // namespace platform
} // namespace orion
