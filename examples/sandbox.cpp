#include <orion/orion.h>

class SandboxApp final : public orion::Application
{
    void on_update() override
    {
        exit();
    }

    void on_render() override
    {
    }
};

std::unique_ptr<orion::Application> create_orion_app(std::span<const char* const> args)
{
    return std::make_unique<SandboxApp>();
}
