#include "orion-engine/application.h"

#ifndef ORION_APPLICATION_LOG_LEVEL
    #define ORION_APPLICATION_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

#include "orion-utils/assertion.h"

namespace orion
{
    Application::Application()
        : platform_app_(platform::create_application(this), &platform::destroy_application)
        , logger_(orion::create_logger("orion-application", ORION_APPLICATION_LOG_LEVEL))
    {
        SPDLOG_LOGGER_INFO(logger(), "<Orion Engine> version: {}, platform: {}, build_type: {}",
                           ORION_VERSION, ORION_PLATFORM, ORION_BUILD_TYPE);
    }

    void Application::on_update(FrameTime dt)
    {
        on_user_update(dt);
    }

    void Application::on_render()
    {
        on_user_render();
    }

    bool Application::should_exit() const noexcept
    {
        return should_exit_;
    }

    void Application::run()
    {
        ORION_ASSERT(platform_app_ != nullptr);
        platform::run_application(platform_app_.get());
    }

    void Application::exit_application()
    {
        should_exit_ = true;
    }
} // namespace orion
