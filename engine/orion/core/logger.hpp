#pragma once

#define SPDLOG_ACTIVE_LEVEL 0
#include <spdlog/spdlog.h>

namespace orion
{
    enum class LogChannel {
        Core,
        Application,

        COUNT,
    };

    class Logger
    {
    public:
        Logger();
        Logger(const Logger&)            = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&)                 = delete;
        Logger& operator=(Logger&&)      = delete;
        ~Logger()                        = default;

        [[nodiscard]] static std::shared_ptr<spdlog::logger> get(LogChannel channel);
    };
} // namespace orion

#define ORION_LOG_TRACE(channel, ...) SPDLOG_LOGGER_TRACE(::orion::Logger::get(channel), __VA_ARGS__)
#define ORION_LOG_DEBUG(channel, ...) SPDLOG_LOGGER_DEBUG(::orion::Logger::get(channel), __VA_ARGS__)
#define ORION_LOG_INFO(channel, ...)  SPDLOG_LOGGER_INFO(::orion::Logger::get(channel), __VA_ARGS__)
#define ORION_LOG_WARN(channel, ...)  SPDLOG_LOGGER_WARN(::orion::Logger::get(channel), __VA_ARGS__)
#define ORION_LOG_ERROR(channel, ...) SPDLOG_LOGGER_ERROR(::orion::Logger::get(channel), __VA_ARGS__)
