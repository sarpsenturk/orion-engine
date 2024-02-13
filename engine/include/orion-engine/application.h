#pragma once

#include "orion-core/clock.h"

#include <memory>
#include <spdlog/logger.h>

namespace orion
{
    // Forward declare
    class Application;

    // Platform specific application class
    class PlatformApplication;
    namespace platform
    {
        [[nodiscard]] PlatformApplication* create_application(Application* this_ptr);
        void destroy_application(PlatformApplication* application);
        void run_application(PlatformApplication* application);
    } // namespace platform

    using PlatformApplicationPtr = std::unique_ptr<PlatformApplication, decltype(&platform::destroy_application)>;

    // Main application clas for Orion. Entry point for the engine
    class Application
    {
    public:
        Application();
        virtual ~Application() = default;

        void on_update(FrameTime dt);
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
        virtual void on_user_update(FrameTime dt) = 0;
        virtual void on_user_render() = 0;

        PlatformApplicationPtr platform_app_;
        std::shared_ptr<spdlog::logger> logger_;
        bool should_exit_ = false;
    };
} // namespace orion

// Helper macro to subscribe to events
#define ORION_EXIT_APP_FN [this](const auto&) { exit_application(); }
