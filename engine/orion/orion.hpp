#pragma once

#include <memory>

namespace orion
{
    class Application
    {
    public:
        Application()          = default;
        virtual ~Application() = default;

        void update(float dt);
        void render();

    private:
        virtual void on_update(float dt) = 0;
        virtual void on_render()         = 0;
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

    private:
        std::unique_ptr<class Logger> m_logger;
    };
} // namespace orion
