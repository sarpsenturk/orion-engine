#pragma once

#include <memory>

namespace orion
{
    class Application
    {
    public:
        void update();
        void render();

        [[nodiscard]] virtual bool should_exit() const = 0;

    private:
        virtual void on_update() = 0;
        virtual void on_render() = 0;
    };

    class Engine
    {
    public:
        void run(std::unique_ptr<Application> app);
    };
} // namespace orion
