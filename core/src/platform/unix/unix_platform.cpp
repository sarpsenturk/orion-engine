#include "orion-core/platform/unix/unix_platform.h"

#include "orion-utils/assertion.h"

#include <cstring>

namespace orion::unix
{
    const char* errno_string(int error_code)
    {
        const char* string = strerror(error_code);
        ORION_ENSURES(string != nullptr && "error_code was invalid");
        return string;
    }
} // namespace orion::unix
