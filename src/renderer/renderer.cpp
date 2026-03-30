#include "orion/renderer/renderer.hpp"

#include "vulkan_impl.hpp"

#include "orion/window.hpp"

namespace orion
{
    struct Renderer::Impl {
        VulkanInstance vulkan_instance;
        VulkanDevice vulkan_device;
        VulkanSurface vulkan_surface;
        VulkanSwapchain vulkan_swapchain;
    };

    tl::expected<Renderer, std::string> Renderer::initialize(const RendererDesc& desc)
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

        // Create window surface
        auto vulkan_surface = vulkan_device->create_surface(desc.window);
        if (!vulkan_surface) {
            return tl::unexpected("Failed to create Vulkan surface");
        }

        // Create swapchain
        auto vulkan_swapchain = vulkan_device->create_swapchain({
            .surface = *vulkan_surface,
            .requested_extent = {
                static_cast<std::uint32_t>(desc.window.width()),
                static_cast<std::uint32_t>(desc.window.height()),
            },
            .requested_image_count = 2,
            .requested_image_format = VK_FORMAT_B8G8R8A8_SRGB,
            .requested_present_mode = VK_PRESENT_MODE_FIFO_KHR,
        });
        if (!vulkan_swapchain) {
            return tl::unexpected("Failed to create Vulkan swapchain");
        }

        return Renderer{std::make_unique<Impl>(
            std::move(*vulkan_instance),
            std::move(*vulkan_device),
            std::move(*vulkan_surface),
            std::move(*vulkan_swapchain))};
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
