#pragma once

#include "orion-core/dl_lib.h"
#include "win32_headers.h"

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
