#include "orion/core/engine.h"

#include "orion/config.h"

#include <arg-parse/arg-parse.h>
#include <spdlog/spdlog.h>

namespace orion
{

    Engine::Engine(Application::Ptr application, int argc, const char* argv[])
        : application_(std::move(application))
    {
        auto cmd_args = argparse::ArgParse(argc, argv);
        spdlog::info("Orion Engine Version {}.{}.{} Build Type: {}", ORION_VERSION_MAJOR,
                     ORION_VERSION_MINOR, ORION_VERSION_PATCH, ORION_BUILD_DEBUG);
        application_->on_create(cmd_args);
    }

    int Engine::main()
    {
        while (!should_close()) {
            on_render();
            on_update();
        }
        application_->on_shutdown();
        return 0;
    }

    bool Engine::should_close() const noexcept
    {
        return application_->should_close();
    }

    void Engine::on_update() {}

    void Engine::on_render() { application_->on_update(); }
} // namespace orion
