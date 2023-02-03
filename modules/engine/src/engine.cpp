#include "orion-engine/engine.h"

namespace orion
{
    Engine::Engine(std::unique_ptr<Application> application)
        : application_(std::move(application))
    {
    }

    void Engine::main_loop()
    {
        while (!application_->should_exit()) {
            update();
            render();
        }
    }

    void Engine::update()
    {
        application_->on_update();
    }

    void Engine::render()
    {
        application_->on_render();
    }
} // namespace orion
