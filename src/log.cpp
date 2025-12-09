#include "orion/log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace orion
{
    std::shared_ptr<spdlog::logger> Log::core_logger;
    std::shared_ptr<spdlog::logger> Log::app_logger;

    bool Log::init()
    {
        core_logger = spdlog::stdout_color_mt("core");
        core_logger->set_level(spdlog::level::trace);

        app_logger = spdlog::stdout_color_mt("app");
        app_logger->set_level(spdlog::level::trace);

        return true;
    }

    std::shared_ptr<spdlog::logger> Log::get_core_logger()
    {
        return core_logger;
    }

    std::shared_ptr<spdlog::logger> Log::get_app_logger()
    {
        return app_logger;
    }
} // namespace orion
