#include <orion/application.h>

using namespace orion;

class SandboxApp final : public Application
{
    void on_update() override
    {
        exit();
    }

    void on_render() override
    {
    }
};

std::unique_ptr<Application> orion_main(std::span<const char* const> args)
{
    return std::make_unique<SandboxApp>();
}
