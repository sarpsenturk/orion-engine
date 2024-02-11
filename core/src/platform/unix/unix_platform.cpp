#include "orion-core/platform/unix/unix_platform.h"

#ifndef ORION_UNIX_LOG_LEVEL
    #define ORION_UNIX_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"

#include "orion-utils/assertion.h"

#include <cerrno>
#include <cstring>

namespace orion::unix
{
    unexpected<int> errno_unexpect()
    {
        return unexpected<int>(errno);
    }

    const char* errno_string(int error_code)
    {
        const char* string = strerror(error_code);
        ORION_ENSURES(string != nullptr && "error_code was invalid");
        return string;
    }

    const char* errno_string()
    {
        return errno_string(errno);
    }

    spdlog::logger* logger()
    {
        static const auto logger = create_logger("unix-logger", ORION_UNIX_LOG_LEVEL);
        return logger.get();
    }
} // namespace orion::unix
