#include "orion/renderer/renderer.hpp"

#include "vulkan_impl.hpp"

namespace orion
{
    struct Renderer::Impl {
        VulkanInstance vulkan_instance;
        VulkanDevice vulkan_device;
    };

    tl::expected<Renderer, std::string> Renderer::initialize()
    {
        // Create vulkan instance
        auto vulkan_instance = VulkanInstance::create();
        if (!vulkan_instance) {
            return tl::unexpected("Failed to create Vulkan instance");
        }

        // Get list of available GPUs
        const auto physical_devices = vulkan_instance->enumerate_physical_devices();
        if (!physical_devices) {
            return tl::unexpected("Failed to enumerate physical devices");
        }

        // Create device
        // Use first available GPU for now
        // TODO: Allow user to select this
        VkPhysicalDevice physical_device = (*physical_devices)[0];
        auto vulkan_device = vulkan_instance->create_device(physical_device);
        if (!vulkan_device) {
            return tl::unexpected("Failed to create Vulkan device");
        }

        return Renderer{std::make_unique<Impl>(
            std::move(*vulkan_instance),
            std::move(*vulkan_device))};
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
