#include "orion-core/log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace orion
{
    std::shared_ptr<spdlog::logger> create_logger(const char* name, spdlog::level::level_enum log_level, const char* format)
    {
        auto logger = spdlog::stdout_color_st(name);
        logger->set_pattern(format);
        logger->set_level(static_cast<spdlog::level::level_enum>(log_level));
        return logger;
    }
} // namespace orion
