#include "orion/renderer/renderer.hpp"

#include "vulkan_impl.hpp"

#include "orion/window.hpp"

#include <array>
#include <cstdint>

namespace orion
{
    static constexpr auto frames_in_flight = 2;

    struct PerFrameData {
        VulkanCommandPool vulkan_command_pool;

        VulkanSemaphore image_available_semaphore;
        VulkanSemaphore render_complete_semaphore;
    };

    struct Renderer::Impl {
        VulkanInstance vulkan_instance;
        VulkanDevice vulkan_device;
        VulkanSurface vulkan_surface;
        VulkanSwapchain vulkan_swapchain;

        std::uint64_t frame_count = 0;
        std::array<PerFrameData, frames_in_flight> frame_data;
        VulkanSemaphore frame_semaphore;

        Impl(
            VulkanInstance instance,
            VulkanDevice device,
            VulkanSurface surface,
            VulkanSwapchain swapchain,
            std::array<PerFrameData, frames_in_flight> frame_data,
            VulkanSemaphore frame_semaphore)
            : vulkan_instance(std::move(instance))
            , vulkan_device(std::move(device))
            , vulkan_surface(std::move(surface))
            , vulkan_swapchain(std::move(swapchain))
            , frame_data(std::move(frame_data))
            , frame_semaphore(std::move(frame_semaphore))
        {
        }
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

        // Create per frame resources
        std::array<PerFrameData, frames_in_flight> frame_data;
        for (int i = 0; i < frames_in_flight; ++i) {
            auto command_pool = vulkan_device->create_command_pool({vulkan_device->graphics_queue_family, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT});
            if (!command_pool) {
                return tl::unexpected("Failed to create Vulkan command pool");
            }
            frame_data[i].vulkan_command_pool = std::move(*command_pool);

            auto image_available_semaphore = vulkan_device->create_binary_semaphore();
            if (!image_available_semaphore) {
                return tl::unexpected("Failed to create Vulkan semaphpre");
            }
            frame_data[i].image_available_semaphore = std::move(*image_available_semaphore);
            auto render_complete_semaphore = vulkan_device->create_binary_semaphore();
            if (!render_complete_semaphore) {
                return tl::unexpected("Failed to create Vulkan semaphpre");
            }
            frame_data[i].render_complete_semaphore = std::move(*render_complete_semaphore);
        }

        // Create frame counter semaphore
        auto frame_semaphore = vulkan_device->create_timeline_semaphore(0);
        if (!frame_semaphore) {
            return tl::unexpected("Failed to create Vulkan semaphore");
        }

        return Renderer{std::make_unique<Impl>(
            std::move(*vulkan_instance),
            std::move(*vulkan_device),
            std::move(*vulkan_surface),
            std::move(*vulkan_swapchain),
            std::move(frame_data),
            std::move(*frame_semaphore))};
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
