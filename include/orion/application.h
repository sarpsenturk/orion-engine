#pragma once

#include <memory>
#include <span>
#include <string>

namespace orion
{
    class Application
    {
    public:
        Application();
        virtual ~Application() = default;

        void update();
        void render();

        [[nodiscard]] bool should_exit() const { return should_exit_; }

    protected:
        Application(Application&&) = default;
        Application& operator=(Application&&) = default;

        void exit_application();
        [[noreturn]] void abort_application(const std::string& what);

    private:
        virtual void on_update() = 0;
        virtual void on_render() = 0;

        bool should_exit_ = false;
    };
} // namespace orion

std::unique_ptr<orion::Application> orion_main(std::span<const char* const> args);
