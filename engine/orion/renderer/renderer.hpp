#pragma once

#include "orion/renderer/gpu.hpp"

namespace orion
{
    class Renderer
    {
    public:
        Renderer();
        Renderer(const Renderer&)            = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&)                 = delete;
        Renderer& operator=(Renderer&&)      = delete;
        ~Renderer();

    private:
        gpu::Device* m_device;
    };
} // namespace orion
