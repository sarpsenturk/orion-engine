#include "renderer.hpp"

namespace orion
{
    Renderer::Renderer()
        : m_device(gpu::create_device(gpu::DeviceDesc{}), gpu::destroy_device)
    {
    }

    Renderer::~Renderer() = default;
} // namespace orion
