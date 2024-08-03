#pragma once

#include <memory>
#include <span>

namespace orion
{
    class Application
    {
    public:
        Application() = default;
        virtual ~Application() = default;

        void update();
        void render();

        [[nodiscard]] bool should_exit() const { return should_exit_; }

    protected:
        Application(const Application&) = default;
        Application(Application&&) = default;
        Application& operator=(const Application&) = default;
        Application& operator=(Application&&) = default;

        void exit();

    private:
        virtual void on_update() = 0;
        virtual void on_render() = 0;

        bool should_exit_ = false;
    };
} // namespace orion

std::unique_ptr<orion::Application> create_orion_app(std::span<const char* const> args);
