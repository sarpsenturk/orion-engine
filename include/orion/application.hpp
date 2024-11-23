#pragma once

#include <chrono>
#include <memory>
#include <span>
#include <string>

namespace orion
{
    class Application
    {
    public:
        using clock = std::chrono::high_resolution_clock;
        using time_point = clock::time_point;
        using duration = std::chrono::duration<float>;

        Application();
        virtual ~Application() = default;

        void update();
        void render();

        [[nodiscard]] bool should_exit() const { return should_exit_; }

    protected:
        Application(Application&&) = default;
        Application& operator=(Application&&) = default;

        void orion_exit();
        [[noreturn]] void orion_abort(const std::string& what);

    private:
        virtual void on_update(duration dt) = 0;
        virtual void on_render() = 0;

        bool should_exit_ = false;

        time_point last_update_ = {};
    };
} // namespace orion

std::unique_ptr<orion::Application> orion_main(std::span<const char* const> args);
