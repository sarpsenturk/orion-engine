#pragma once

#include "orion/gpu/gpu.hpp"

#include <memory>

namespace orion
{
    class Renderer
    {
    public:
        Renderer();
        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&)                 = default;
        Renderer& operator=(Renderer&&)      = default;
        ~Renderer();

    private:
        std::unique_ptr<gpu::Device, void (*)(gpu::Device*)> m_device;
    };
} // namespace orion
