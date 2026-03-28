#include "orion/renderer/renderer.hpp"

#include "vulkan_impl.hpp"

namespace orion
{
    struct Renderer::Impl {
        VulkanInstance vulkan_instance;
    };

    tl::expected<Renderer, std::string> Renderer::initialize()
    {
        // Create vulkan instance
        auto vulkan_instance = VulkanInstance::create();
        if (!vulkan_instance) {
            return tl::unexpected("Failed to create Vulkan instance");
        }

        return Renderer{std::make_unique<Impl>(std::move(*vulkan_instance))};
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
