#pragma once

#include <spdlog/spdlog.h>

namespace orion
{
    class Logger
    {
    public:
        Logger();
        ~Logger();

        static std::shared_ptr<spdlog::logger> get_core_logger() { return core_logger; }

    private:
        static std::shared_ptr<spdlog::logger> core_logger;
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
