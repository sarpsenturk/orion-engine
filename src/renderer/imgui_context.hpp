#pragma once

#include "vulkan_impl.hpp"

#include <tl/expected.hpp>

#include <string>

namespace orion
{
    struct ImGuiContextWrapperDesc {
        const class Window& window;
        const VulkanDevice& vulkan_device;
        const VulkanSwapchain& vulkan_swapchain;
    };

    class ImGuiContextWrapper
    {
    public:
        ImGuiContextWrapper(const ImGuiContextWrapper&) = delete;
        ImGuiContextWrapper& operator=(const ImGuiContextWrapper&) = delete;
        ImGuiContextWrapper(ImGuiContextWrapper&& other) noexcept;
        ImGuiContextWrapper& operator=(ImGuiContextWrapper&& other) noexcept;
        ~ImGuiContextWrapper();

        static tl::expected<ImGuiContextWrapper, std::string> create(const ImGuiContextWrapperDesc& desc);

    private:
        ImGuiContextWrapper() = default;
        int sentinel_ = 42;
    };
} // namespace orion
