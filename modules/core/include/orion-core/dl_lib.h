#pragma once

#include <memory> // std::unique_ptr
#include <string> // std::string

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

    class Module
    {
    public:
        explicit Module(std::string filename);

        [[nodiscard]] auto& filename() const noexcept { return filename_; }
        [[nodiscard]] auto* platform_module() const noexcept { return platform_module_.get(); }

        template<typename T>
        [[nodiscard]] T* load_symbol(const char* symbol_name) const noexcept
        {
            return reinterpret_cast<T*>(platform::load_library_address(platform_module_.get(), symbol_name));
        }

    private:
        PlatformModulePtr platform_module_;

        std::string filename_;
    };

    static_assert(std::is_same_v<decltype(std::declval<Module>().load_symbol<void(int)>("test")), void (*)(int)>);
} // namespace orion
