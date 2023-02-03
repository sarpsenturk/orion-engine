#include "orion-engine/engine.h"

#include "orion-core/config.h"

#include <spdlog/spdlog.h> // SPDLOG_*

namespace orion
{
    Engine::Engine(std::unique_ptr<Application> application)
        : application_(std::move(application))
    {
        spdlog::set_pattern("[%^%l%$] %v");
        SPDLOG_INFO("Orion Engine {}", current_platform);
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
