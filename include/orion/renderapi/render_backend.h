#pragma once

#include "orion/renderapi/render_device.h"

#include <memory>
#include <string>
#include <vector>

namespace orion
{
    struct GraphicsAdapter {
        std::string name;
    };

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

        // Gets the adapters (GPUs) available on the system
        std::vector<GraphicsAdapter> get_adapters();

        // Create a logical device
        std::unique_ptr<RenderDevice> create_device(std::size_t adapter_index);

    protected:
        RenderBackend(const RenderBackend&) = default;
        RenderBackend& operator=(const RenderBackend&) = default;
        RenderBackend(RenderBackend&&) = default;
        RenderBackend& operator=(RenderBackend&&) = default;

    private:
        virtual std::vector<GraphicsAdapter> get_adapters_api() = 0;
        virtual std::unique_ptr<RenderDevice> create_device_api(std::size_t adapter_index) = 0;
    };
} // namespace orion
