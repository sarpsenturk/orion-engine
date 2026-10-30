#include "logger.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

#include <array>
#include <utility>

namespace orion
{
    static constexpr auto channels = std::array{
        "CORE",
        "PLATFORM",
        "APP",
        "ASSERT",
        "GPU",
        "RENDERER",
    };
    static_assert(channels.size() == std::to_underlying(LogChannel::COUNT));

    Logger::Logger()
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        for (const char* channel : channels) {
            auto logger = std::make_shared<spdlog::logger>(channel, console_sink);
            logger->set_level(spdlog::level::trace);
            spdlog::register_logger(std::move(logger));
        }
    }

    std::shared_ptr<spdlog::logger> Logger::get(LogChannel channel)
    {
        return spdlog::get(channels[std::to_underlying(channel)]);
    }
} // namespace orion
