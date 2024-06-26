#pragma once

#include "win32_platform.h"

namespace orion
{
    class PlatformModule
    {
    public:
        explicit PlatformModule(HMODULE hmodule);

        [[nodiscard]] auto hmodule() const noexcept { return hmodule_; }

    private:
        HMODULE hmodule_;
    };
} // namespace orion
