#pragma once

#include "handles.h"

#include <cstddef> // std::byte
#include <span>    // std::span

namespace orion
{
    struct ShaderModuleDesc {
        std::span<const std::byte> byte_code;
    };

    class ShaderModule
    {
    public:
        ShaderModule(ShaderModuleHandle handle);

        [[nodiscard]] auto handle() const noexcept { return handle_; }

    private:
        ShaderModuleHandle handle_;
    };
} // namespace orion
