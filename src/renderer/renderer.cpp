#include "orion/renderer/renderer.hpp"

namespace orion
{
    struct Renderer::Impl {
    };

    tl::expected<Renderer, std::string> Renderer::initialize()
    {
        return Renderer{std::make_unique<Impl>()};
    }

    Renderer::Renderer(std::unique_ptr<Impl> impl)
        : impl_(std::move(impl))
    {
    }
    // Need to explicitly defaut here where Impl is defined
    Renderer::Renderer(Renderer&&) noexcept = default;
    Renderer& Renderer::operator=(Renderer&&) noexcept = default;
    Renderer::~Renderer() = default;
} // namespace orion
