#include "orion/log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace orion
{
    std::shared_ptr<spdlog::logger> Logger::core_logger;

    Logger::Logger()
    {
        core_logger = spdlog::stdout_color_mt("core");
        core_logger->set_level(spdlog::level::trace);
    }

    Logger::~Logger()
    {
        core_logger = nullptr;
    }
} // namespace orion
