#pragma once

#include <spdlog/logger.h>

namespace orion
{
    inline constexpr auto default_log_format = "[%n] [%^%l%$] %v";

    std::shared_ptr<spdlog::logger> create_logger(const char* name,
                                                  spdlog::level::level_enum log_level,
                                                  const char* format = default_log_format);
} // namespace orion
