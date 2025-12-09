#include "orion/orion.hpp"

#include "orion/log.hpp"
#include "orion/platform.hpp"

#include <stdexcept>

namespace orion
{
    Engine::Engine()
    {
        if (!Log::init()) {
            throw std::runtime_error("Failed to initialize log");
        }
        ORION_CORE_LOG_INFO("Orion Engine {} {} {}", ORION_OS_NAME, ORION_ARCH_NAME, ORION_COMPILER_NAME);
    }

    void Engine::run(std::unique_ptr<Application> app)
    {
        while (!app->should_close()) {
            app->update();
            app->render();
        }
    }
} // namespace orion
