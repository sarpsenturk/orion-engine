#include <orion/application.hpp>
#include <orion/window.hpp>

#include <orion/renderapi/render_backend.hpp>

#include <orion/renderer/renderer.hpp>
#include <orion/renderer/sprite_renderer.hpp>

#include <spdlog/spdlog.h>

using namespace orion;

class SandboxApp final : public Application
{
public:
    SandboxApp()
        : window_({.title = "Sandbox App", .width = 800, .height = 600})
        , renderer_({.render_backend = RenderBackend::create_builtin_vulkan(), .width = 800, .height = 600})
        , sprite_renderer_({.render_device = renderer_.render_device(), .screen_width = 800, .screen_height = 600, .camera_size = 100})
    {
        // Get render device from renderer
        auto* render_device = renderer_.render_device();

        // Get command queue from renderer
        auto* graphics_queue = renderer_.graphics_queue();

        // Create swapchain
        swapchain_ = render_device->create_swapchain({.window = &window_, .queue = graphics_queue, .width = 800, .height = 600, .image_count = 2, .image_format = Format::B8G8R8A8_Unorm});
    }

private:
    void on_update() override
    {
        WindowEvent event;
        while ((event = window_.poll_event())) {
            SPDLOG_TRACE("{}", event);
            if (event.is<OnWindowClose>()) {
                orion_exit();
            }
        }
    }

    void on_render() override
    {
        // Begin a new frame
        renderer_.begin_frame();

        // Draw sprite
        sprite_renderer_.draw();

        // Submit sprite
        sprite_renderer_.submit(&renderer_);

        // End frame
        renderer_.end_frame();

        // Present results to swapchain
        renderer_.present(swapchain_.get(), 800, 600);
    }

    Window window_;
    Renderer renderer_;
    SpriteRenderer sprite_renderer_;
    std::unique_ptr<Swapchain> swapchain_;
};

std::unique_ptr<Application> orion_main(std::span<const char* const> args)
{
    return std::make_unique<SandboxApp>();
}
