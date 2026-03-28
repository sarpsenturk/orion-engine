#include "orion/log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

#include <utility>

namespace orion
{
    std::shared_ptr<spdlog::logger> Logger::core_logger;
    std::shared_ptr<spdlog::logger> Logger::renderer_logger;

    tl::expected<Logger, std::string> Logger::initialize()
    {
        core_logger = spdlog::stdout_color_mt("core");
        core_logger->set_level(spdlog::level::trace);
        renderer_logger = spdlog::stdout_color_mt("renderer");
        renderer_logger->set_level(spdlog::level::trace);
        return Logger{};
    }

    Logger::Logger(Logger&& other) noexcept
        : sentinel_(std::exchange(other.sentinel_, 0))
    {
    }

    Logger& Logger::operator=(Logger&& other) noexcept
    {
        if (this != &other) {
            sentinel_ = std::exchange(other.sentinel_, 0);
        }
        return *this;
    }

    Logger::~Logger()
    {
        if (sentinel_ != 0) {
            core_logger = nullptr;
            renderer_logger = nullptr;
        }
    }
} // namespace orion
