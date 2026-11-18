#pragma once

#define ORION_LOG_LEVEL_TRACE 0
#define ORION_LOG_LEVEL_DEBUG 1
#define ORION_LOG_LEVEL_INFO  2
#define ORION_LOG_LEVEL_WARN  3
#define ORION_LOG_LEVEL_ERROR 4
#define ORION_LOG_LEVEL_OFF   6

#ifndef ORION_ACTIVE_LOG_LEVEL
    #define ORION_ACTIVE_LOG_LEVEL ORION_LOG_LEVEL_INFO
#endif

#include <format>
#include <string>

namespace orion
{
    namespace log
    {
        enum class Level {
            Trace = ORION_LOG_LEVEL_TRACE,
            Debug = ORION_LOG_LEVEL_DEBUG,
            Info  = ORION_LOG_LEVEL_INFO,
            Warn  = ORION_LOG_LEVEL_WARN,
            Error = ORION_LOG_LEVEL_ERROR,
            Off   = ORION_LOG_LEVEL_OFF,
        };

        // Initialize a new logging channel
        void add_channel(const std::string& name, log::Level level = Level::Info);

        // Checks if a channel currently exists
        [[nodiscard]] bool has_channel(const std::string& name);

        // Write the contents of msg at severity level to channel
        void write(const std::string& channel, Level level, const std::string& msg);

        template<typename... Args>
        void trace(const std::string& channel, std::format_string<Args...> fmt, Args&&... args)
        {
            write(channel, Level::Trace, std::format(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void debug(const std::string& channel, std::format_string<Args...> fmt, Args&&... args)
        {
            write(channel, Level::Debug, std::format(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void info(const std::string& channel, std::format_string<Args...> fmt, Args&&... args)
        {
            write(channel, Level::Info, std::format(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void warn(const std::string& channel, std::format_string<Args...> fmt, Args&&... args)
        {
            write(channel, Level::Warn, std::format(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void error(const std::string& channel, std::format_string<Args...> fmt, Args&&... args)
        {
            write(channel, Level::Error, std::format(fmt, std::forward<Args>(args)...));
        }
    } // namespace log

    struct LoggingPlugin {
        void build(class App& app);
    };
} // namespace orion

#if ORION_ACTIVE_LOG_LEVEL <= ORION_LOG_LEVEL_TRACE
    #define ORION_LOG_TRACE(channel, ...) ::orion::log::trace(channel, __VA_ARGS__)
#else
    #define ORION_LOG_TRACE(channel, ...) ((void)0)
#endif

#if ORION_ACTIVE_LOG_LEVEL <= ORION_LOG_LEVEL_DEBUG
    #define ORION_LOG_DEBUG(channel, ...) ::orion::log::debug(channel, __VA_ARGS__)
#else
    #define ORION_LOG_DEBUG(channel, ...) ((void)0)
#endif

#if ORION_ACTIVE_LOG_LEVEL <= ORION_LOG_LEVEL_INFO
    #define ORION_LOG_INFO(channel, ...) ::orion::log::info(channel, __VA_ARGS__)
#else
    #define ORION_LOG_INFO(channel, ...) ((void)0)
#endif

#if ORION_ACTIVE_LOG_LEVEL <= ORION_LOG_LEVEL_WARN
    #define ORION_LOG_WARN(channel, ...) ::orion::log::warn(channel, __VA_ARGS__)
#else
    #define ORION_LOG_WARN(channel, ...) ((void)0)
#endif

#if ORION_ACTIVE_LOG_LEVEL <= ORION_LOG_LEVEL_ERROR
    #define ORION_LOG_ERROR(channel, ...) ::orion::log::error(channel, __VA_ARGS__)
#else
    #define ORION_LOG_ERROR(channel, ...) ((void)0)
#endif
