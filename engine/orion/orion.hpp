#pragma once

#include "orion/core/event.hpp"

#include <memory>

namespace orion
{
    class Application
    {
    public:
        Application()          = default;
        virtual ~Application() = default;

        void update();
        void render();
        void event(const Event& event);

        [[nodiscard]] virtual bool should_exit() const = 0;

    private:
        virtual void on_update()                  = 0;
        virtual void on_render()                  = 0;
        virtual void on_event(const Event& event) = 0;
    };

    class Engine
    {
    public:
        Engine();
        Engine(const Engine&)            = delete;
        Engine& operator=(const Engine&) = delete;
        Engine(Engine&&)                 = delete;
        Engine& operator=(Engine&&)      = delete;
        ~Engine();

        void run(std::unique_ptr<Application> app);

        [[nodiscard]] class Renderer& renderer() { return *m_renderer; }

    private:
        void on_event(const Event& event);

        std::unique_ptr<class Logger> m_logger;
        std::unique_ptr<class Platform> m_platform;
        std::unique_ptr<class Renderer> m_renderer;
    };
} // namespace orion
