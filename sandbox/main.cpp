#include "orion-engine/orion-engine.h"

class SandboxApp : public orion::Application
{
    void on_user_update() override
    {
    }

    void on_user_render() override
    {
    }

    [[nodiscard]] bool user_should_exit() const noexcept override
    {
        return true;
    }
};

ORION_MAIN(args)
{
    orion::Engine engine(std::make_unique<SandboxApp>());
    engine.main_loop();
    return 0;
}
