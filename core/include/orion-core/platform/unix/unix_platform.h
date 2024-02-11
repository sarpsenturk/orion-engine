#pragma once

#include "orion-utils/expected.h"

namespace spdlog
{
    class logger;
}

namespace orion::unix
{
    // A syscall wrapper returning an expected type and unexpected errno
    template<typename Expected>
    using SyscallResult = expected<Expected, int>;

    [[nodiscard]] unexpected<int> errno_unexpect();

    [[nodiscard]] const char* errno_string(int error_code);
    [[nodiscard]] const char* errno_string();

    [[nodiscard]] spdlog::logger* logger();
}
