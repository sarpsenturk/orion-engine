#include "renderer.hpp"

namespace orion
{
    Renderer::Renderer()
        : m_device(gpu::create_device({}))
    {
    }

    Renderer::~Renderer()
    {
        gpu::destroy_device(m_device);
    }
} // namespace orion
