#pragma once

#include "orion-core/dyn_lib.h"

namespace orion
{
    class PlatformModule
    {
    public:
        explicit PlatformModule(void* handle);

        [[nodiscard]] void* handle() const noexcept { return handle_; }

    private:
        void* handle_;
    };
} // namespace orion
