#pragma once

#include <spdlog/spdlog.h>

namespace orion
{
    class Log
    {
    public:
        static bool init();

        static std::shared_ptr<spdlog::logger> get_core_logger();
        static std::shared_ptr<spdlog::logger> get_app_logger();

    private:
        static std::shared_ptr<spdlog::logger> core_logger;
        static std::shared_ptr<spdlog::logger> app_logger;
    };
} // namespace orion

#define ORION_CORE_LOG_TRACE(...) ::orion::Log::get_core_logger()->trace(__VA_ARGS__)
#define ORION_CORE_LOG_DEBUG(...) ::orion::Log::get_core_logger()->debug(__VA_ARGS__)
#define ORION_CORE_LOG_INFO(...) ::orion::Log::get_core_logger()->info(__VA_ARGS__)
#define ORION_CORE_LOG_WARN(...) ::orion::Log::get_core_logger()->warn(__VA_ARGS__)
#define ORION_CORE_LOG_ERROR(...) ::orion::Log::get_core_logger()->error(__VA_ARGS__)

#define ORION_APP_LOG_TRACE(...) ::orion::Log::get_app_logger()->trace(__VA_ARGS__)
#define ORION_APP_LOG_DEBUG(...) ::orion::Log::get_app_logger()->debug(__VA_ARGS__)
#define ORION_APP_LOG_INFO(...) ::orion::Log::get_app_logger()->info(__VA_ARGS__)
#define ORION_APP_LOG_WARN(...) ::orion::Log::get_app_logger()->warn(__VA_ARGS__)
#define ORION_APP_LOG_ERROR(...) ::orion::Log::get_app_logger()->error(__VA_ARGS__)
