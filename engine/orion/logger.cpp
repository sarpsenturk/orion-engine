#include "logger.hpp"

#include "orion/app.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace orion
{
    namespace log
    {
        static spdlog::level::level_enum to_spdlog_level(Level level)
        {
            switch (level) {
                case Level::Trace:
                    return spdlog::level::trace;
                case Level::Debug:
                    return spdlog::level::debug;
                case Level::Info:
                    return spdlog::level::info;
                case Level::Warn:
                    return spdlog::level::warn;
                case Level::Error:
                    return spdlog::level::err;
                case Level::Off:
                    return spdlog::level::off;
            }
        }

        void add_channel(const std::string& name, log::Level level)
        {
            const auto logger = spdlog::stdout_color_mt(name);
            logger->set_level(to_spdlog_level(level));
        }

        void write(const std::string& channel, Level level, const std::string& msg)
        {
            const auto logger = spdlog::get(channel);
            if (!logger) { return; };
            logger->log(to_spdlog_level(level), msg);
        }
    } // namespace log

    void LoggingPlugin::build(App&)
    {
    }
} // namespace orion
