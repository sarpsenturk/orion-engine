#pragma once

#include <memory>

namespace orion
{
    class RenderBackend
    {
    public:
        RenderBackend() = default;
        virtual ~RenderBackend() = default;

        // Create the builtin Vulkan backend
        static std::unique_ptr<RenderBackend> create_builtin_vulkan();

        // Create the builtin DirectX12 backend
        static std::unique_ptr<RenderBackend> create_builtin_d3d12();

        // Create the builtin default backend for the target platform
        static std::unique_ptr<RenderBackend> create();

        // Gets the name of the backend
        [[nodiscard]] virtual const char* name() const noexcept = 0;

    protected:
        RenderBackend(const RenderBackend&) = default;
        RenderBackend& operator=(const RenderBackend&) = default;
        RenderBackend(RenderBackend&&) = default;
        RenderBackend& operator=(RenderBackend&&) = default;
    };
} // namespace orion
