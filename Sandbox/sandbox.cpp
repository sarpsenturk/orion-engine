#include "orion/orion.h"
#include "orion/window/window.h"

#include <spdlog/spdlog.h>

class Sandbox : public orion::Application
{
public:
    Sandbox()
        : window_({"Sandbox"})
    {
    }

private:
    void on_user_create(const argparse::ArgParse& args) override
    {
        spdlog::info("on_create()");
        window_.initialize();
    }

    void on_user_shutdown() override
    {
        spdlog::info("on_shutdown()");
        window_.destroy();
    }

    void on_user_update() override { window_.update(); }

    bool user_should_close() const noexcept override
    {
        return window_.should_close();
    }

    orion::Window window_;
};

auto orion::create_application() -> std::unique_ptr<Application>
{
    return std::make_unique<Sandbox>();
}
