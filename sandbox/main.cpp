#include <orion-core/window.h>
#include <orion-engine/orion-engine.h>
#include <orion-renderer/renderer.h>

class SandboxApp : public orion::Application
{
public:
    SandboxApp()
        : window_({.name = "Orion Sandbox", .position = {400, 200}, .size = {800, 600}})
        , renderer_(ORION_VULKAN_MODULE)
    {
        // Get the render device
        auto device = renderer_.device();

        // Create swapchain
        swapchain_ = device->create_swapchain(window_, {.image_count = 2, .image_format = orion::Format::B8G8R8A8_SRGB, .image_size = window_.size()});
    }

private:
    void on_user_update() override
    {
        window_.poll_events();
    }

    void on_user_render() override
    {
    }

    [[nodiscard]] bool user_should_exit() const noexcept override
    {
        return window_.should_close();
    }

    orion::Window window_;
    orion::Renderer renderer_;
    orion::Swapchain swapchain_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
