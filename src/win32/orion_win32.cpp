#include "orion_win32.hpp"

namespace orion::win32
{
    std::string format_last_error(DWORD last_error)
    {
        LPSTR buffer = nullptr;
        const auto size = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            last_error,
            0,
            (LPTSTR)&buffer,
            0,
            NULL);
        // This is ugly but it works for now
        try {
            std::string formatted(buffer, size);
            LocalFree(buffer);
            return formatted;
        } catch (...) {
            LocalFree(buffer);
            throw;
        }
    }
} // namespace orion::win32
