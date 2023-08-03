#include <orion-engine/orion-engine.h>

#include <orion-core/window.h>

#include <imgui.h>
#include <orion-renderer/renderer.h>

#include <orion-math/vector/vector3.h>

#include <fmt/chrono.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>

class SandboxApp final : public orion::Application
{
public:
    SandboxApp()
        : window_({.name = "Orion Sandbox", .position = window_position, .size = window_size})
        , renderer_({.device_select_fn = orion::device_select_discrete, .render_size = window_size, .clear_color = {1.f, 0.f, 1.f, 1.f}})
    {
        create_surface();
        create_swapchain();

        window_.on_resize_end() += ([this](const auto&) {
            create_swapchain();
        });

        // Initialize imgui
        renderer_.imgui_init(&window_);
    }

    ~SandboxApp() override
    {
        renderer_.imgui_shutdown();
    }

private:
    void on_user_update([[maybe_unused]] orion::frame_time dt) override
    {
        window_.poll_events();
    }

    void on_user_render() override
    {
        // Begin new frame
        renderer_.begin();

        // Begin imgui frame
        renderer_.imgui_new_frame();

        ImGui::ShowDemoWindow();

        // End imgui frame
        renderer_.imgui_render();

        // End frame
        renderer_.end();

        // Present renderer image to swapchain
        renderer_.present(swapchain_);
    }

    [[nodiscard]] bool user_should_exit() const noexcept override
    {
        return window_.should_close();
    }

    void create_surface()
    {
        surface_ = renderer_.device()->create_surface(window_);
    }

    void create_swapchain()
    {
        if (swapchain_.is_valid()) {
            renderer_.device()->destroy(swapchain_);
        }
        const auto desc = orion::SwapchainDesc{
            .surface = surface_,
            .image_count = swapchain_image_count,
            .image_format = swapchain_image_format,
            .image_size = window_.size(),
            .image_usage = swapchain_image_usage,
        };
        swapchain_ = renderer_.device()->create_swapchain(desc);
    }

    static constexpr auto window_position = orion::WindowPosition{400, 200};
    static constexpr auto window_size = orion::WindowSize{1280, 720};
    static constexpr auto swapchain_image_count = 2;
    static constexpr auto swapchain_image_format = orion::Format::B8G8R8A8_Srgb;
    static constexpr auto swapchain_image_usage = orion::ImageUsageFlags::ColorAttachment | orion::ImageUsageFlags::TransferDst;

    orion::Window window_;
    orion::Renderer renderer_;
    orion::SurfaceHandle surface_;
    orion::SwapchainHandle swapchain_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
