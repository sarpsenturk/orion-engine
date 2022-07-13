#include "orion/orion.h"

#include <spdlog/spdlog.h>

class Sandbox : public orion::Application
{
    void on_user_create() override { spdlog::info("on_create()"); }
    void on_user_shutdown() override { spdlog::info("on_shutdown()"); }
};

auto orion::create_application() -> std::unique_ptr<Application>
{
    return std::make_unique<Sandbox>();
}
