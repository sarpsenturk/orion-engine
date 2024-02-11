#pragma once

namespace orion::unix
{
    [[nodiscard]] const char* errno_string(int error_code);
}
