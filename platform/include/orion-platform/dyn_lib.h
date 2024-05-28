#pragma once

#include <memory>

namespace orion
{
    // Platform specific library handle
    class PlatformModule;

    // Platform specific functions. Implemented per platform
    namespace platform
    {
        PlatformModule* load_library(const char* filename);
        void free_library(PlatformModule* platform_module);
        void* load_library_address(PlatformModule* platform_module, const char* symbol);
    } // namespace platform

    // unique_ptr to platform specific implementation
    using PlatformModulePtr = std::unique_ptr<PlatformModule, decltype(&platform::free_library)>;
} // namespace orion
