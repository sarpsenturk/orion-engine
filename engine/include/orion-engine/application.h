#pragma once

#include "orion-core/clock.h"

#include <memory>
#include <spdlog/logger.h>

namespace orion
{
    class Application
    {
    public:
        Application();
        virtual ~Application() = default;

        void on_update(frame_time dt);
        void on_render();
        [[nodiscard]] bool should_exit() const noexcept;
        void run();

        [[nodiscard]] auto logger() const noexcept { return logger_.get(); }

    protected:
        Application(const Application&) = default;
        Application(Application&&) noexcept = default;
        Application& operator=(const Application&) = default;
        Application& operator=(Application&&) noexcept = default;

        void exit_application();

    private:
        virtual void on_user_update(frame_time dt) = 0;
        virtual void on_user_render() = 0;

        std::shared_ptr<spdlog::logger> logger_;

        bool should_exit_ = false;
    };
} // namespace orion
