#pragma once

#include <spdlog/spdlog.h>
#include <tl/expected.hpp>

namespace orion
{
    class Logger
    {
    public:
        static tl::expected<Logger, std::string> initialize();
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&& other) noexcept;
        Logger& operator=(Logger&& other) noexcept;
        ~Logger();

        static std::shared_ptr<spdlog::logger> get_core_logger() { return core_logger; }

    private:
        static std::shared_ptr<spdlog::logger> core_logger;

        Logger() = default;
        int sentinel_ = 42;
    };
} // namespace orion

#define ORION_CORE_LOG_TRACE(...) ::orion::Logger::get_core_logger()->trace(__VA_ARGS__)
#define ORION_CORE_LOG_DEBUG(...) ::orion::Logger::get_core_logger()->debug(__VA_ARGS__)
#define ORION_CORE_LOG_INFO(...) ::orion::Logger::get_core_logger()->info(__VA_ARGS__)
#define ORION_CORE_LOG_WARN(...) ::orion::Logger::get_core_logger()->warn(__VA_ARGS__)
#define ORION_CORE_LOG_ERROR(...) ::orion::Logger::get_core_logger()->error(__VA_ARGS__)

#define ORION_LOG_EVENT(event) (event).subscribe([](const auto& e) { \
    ORION_CORE_LOG_TRACE("[Event] {}", e);                           \
    return false;                                                    \
})
