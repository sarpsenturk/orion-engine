#include "orion/orion.h"

class Sandbox : public orion::Application
{
    void on_user_create() override { puts("on_create()"); }
    void on_user_shutdown() override { puts("on_shutdown()"); }
};

auto orion::create_application() -> std::unique_ptr<Application>
{
    return std::make_unique<Sandbox>();
}
