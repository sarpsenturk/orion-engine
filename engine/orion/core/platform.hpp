#pragma once

#include "event.hpp"

namespace orion
{
    class Platform
    {
    public:
        Platform();
        Platform(const Platform&)            = delete;
        Platform& operator=(const Platform&) = delete;
        Platform(Platform&&)                 = delete;
        Platform& operator=(Platform&&)      = delete;
        ~Platform();

        void pump_events();
        bool poll_event(Event& event);

        [[nodiscard]] const char* name() const noexcept;
        [[nodiscard]] const char* version() const noexcept;
    };
} // namespace orion
