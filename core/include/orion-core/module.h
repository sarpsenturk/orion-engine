#pragma once

#include "orion-platform/dyn_lib.h"

#include <memory>
#include <string>

#ifdef ORION_PLATFORM_WINDOWS
    #define ORION_EXPORT __declspec(dllexport)
    #define ORION_IMPORT __declspec(dllimport)
#else
    #define ORION_EXPORT __attribute__((visibility(default)))
    #define ORION_IMPORT
#endif

namespace orion
{
    class Module
    {
    public:
        Module();
        explicit Module(std::string filename);

        [[nodiscard]] auto& filename() const noexcept { return filename_; }
        [[nodiscard]] auto* platform_module() const noexcept { return platform_module_.get(); }

        [[nodiscard]] bool is_loaded() const noexcept { return platform_module_ != nullptr; }

        template<typename T>
        [[nodiscard]] T* load_symbol(const char* symbol_name) const noexcept
        {
            return reinterpret_cast<T*>(platform::load_library_address(platform_module_.get(), symbol_name));
        }

    private:
        PlatformModulePtr platform_module_;

        std::string filename_;
    };
} // namespace orion
