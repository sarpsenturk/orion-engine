#pragma once

#ifndef ORION_APPLICATION_LOG_LEVEL
    #define ORION_APPLICATION_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#include <memory>          // std::shared_ptr
#include <spdlog/logger.h> // spdlog::logger

namespace orion
{
    class Application
    {
    public:
        Application();
        virtual ~Application() = default;

        void on_update();
        void on_render();
        [[nodiscard]] bool should_exit() const noexcept;
        void run();

        [[nodiscard]] auto logger() const noexcept { return logger_.get(); }

    protected:
        Application(const Application&) = default;
        Application(Application&&) noexcept = default;
        Application& operator=(const Application&) = default;
        Application& operator=(Application&&) noexcept = default;

    private:
        virtual void on_user_update() = 0;
        virtual void on_user_render() = 0;
        [[nodiscard]] virtual bool user_should_exit() const noexcept = 0;

        std::shared_ptr<spdlog::logger> logger_;
    };
} // namespace orion
