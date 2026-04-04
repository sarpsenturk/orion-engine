#include "imgui_context.hpp"

#include "../platform/platform_glfw.hpp"

#include "orion/log.hpp"

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <utility>

namespace orion
{
    tl::expected<ImGuiContextWrapper, std::string> ImGuiContextWrapper::create(const ImGuiContextWrapperDesc& desc)
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup ImGui platform backend
        if (!ImGui_ImplGlfw_InitForVulkan(desc.window.impl()->window, true)) {
            return tl::unexpected("ImGui_ImplGlfw_InitForVulkan failed");
        }

        // Setup ImGui renderer backend
        auto imgui_info = ImGui_ImplVulkan_InitInfo{
            .ApiVersion = ::orion::vulkan_api_version,
            .Instance = desc.vulkan_device.vk_instance,
            .PhysicalDevice = desc.vulkan_device.vk_physical_device,
            .Device = desc.vulkan_device.vk_device,
            .QueueFamily = desc.vulkan_device.graphics_queue_family,
            .Queue = desc.vulkan_device.graphics_queue,
            .DescriptorPool = VK_NULL_HANDLE,
            .DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE,
            .MinImageCount = desc.vulkan_swapchain.image_count,
            .ImageCount = desc.vulkan_swapchain.image_count,
            .PipelineCache = VK_NULL_HANDLE,
            .PipelineInfoMain = {
                .RenderPass = VK_NULL_HANDLE, // ignored, using dynamic rendering
                .Subpass = 0,
                .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
                .ExtraDynamicStates = {},
                .PipelineRenderingCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                    .pNext = nullptr,
                    .viewMask = 0,
                    .colorAttachmentCount = 1,
                    .pColorAttachmentFormats = &desc.vulkan_swapchain.image_format,
                    .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
                    .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
                },
            },
            .UseDynamicRendering = true,
        };
        if (!ImGui_ImplVulkan_Init(&imgui_info)) {
            return tl::unexpected("ImGui_ImplVulkan_Init failed");
        }
        ORION_RENDERER_LOG_INFO("ImGui initialized: v{}", ImGui::GetVersion());

        return ImGuiContextWrapper{};
    }

    ImGuiContextWrapper::ImGuiContextWrapper(ImGuiContextWrapper&& other) noexcept
        : sentinel_(std::exchange(other.sentinel_, 0))
    {
    }

    ImGuiContextWrapper& ImGuiContextWrapper::operator=(ImGuiContextWrapper&& other) noexcept
    {
        if (this != &other) {
            sentinel_ = std::exchange(other.sentinel_, 0);
        }
        return *this;
    }

    ImGuiContextWrapper::~ImGuiContextWrapper()
    {
        if (sentinel_ != 0) {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            ORION_RENDERER_LOG_INFO("ImGui terminated");
        }
    }
} // namespace orion
