#include <orion-core/window.h>
#include <orion-engine/orion-engine.h>
#include <spdlog/spdlog.h>

class SandboxApp : public orion::Application
{
public:
    SandboxApp()
        : window_({.name = "Orion Sandbox", .position = {400, 200}, .size = {800, 600}})
    {
        window_.on_move_end() += [](const auto& move) { SPDLOG_INFO("{}", move); };
        window_.on_resize_end() += [](const auto& resize) { SPDLOG_INFO("{}", resize); };
        window_.on_close() += [](const auto& close) { SPDLOG_INFO("{}", close); };
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
};

ORION_MAIN(args)
{
    orion::Engine engine(std::make_unique<SandboxApp>());
    engine.main_loop();
    return 0;
}
