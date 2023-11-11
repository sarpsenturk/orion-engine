#pragma once

#include "orion-core/filesystem.h"

#include "win32_platform.h"

namespace orion
{
    class PlatformFile
    {
    public:
        explicit PlatformFile(HANDLE handle);

        [[nodiscard]] HANDLE handle() const noexcept { return handle_; }

    private:
        HANDLE handle_;
    };
} // namespace orion
