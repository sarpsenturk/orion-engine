#pragma once

#include "orion/renderer/gpu.hpp"

namespace orion
{
    struct RendererDesc {
        const class Window* window = nullptr;
    };

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);
        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&)                 = delete;
        Renderer& operator=(Renderer&&)      = delete;
        ~Renderer();

        [[nodiscard]] gpu::Device* device() const { return m_device; }

    private:
        gpu::Device* m_device;
        gpu::Swapchain* m_swapchain;
    };
} // namespace orion
