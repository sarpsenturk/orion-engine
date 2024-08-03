#include <orion/orion.h>

using namespace orion;

class SandboxApp final : public Application
{
    void on_update() override
    {
        cout()->trace("trace");
        cout()->debug("debug");
        cout()->info("info");
        cout()->warn("warn");
        cout()->error("error");
        cerr()->trace("trace");
        cerr()->debug("debug");
        cerr()->info("info");
        cerr()->warn("warn");
        cerr()->error("error");
        exit();
    }

    void on_render() override
    {
    }
};

std::unique_ptr<Application> create_orion_app(std::span<const char* const> args)
{
    return std::make_unique<SandboxApp>();
}
