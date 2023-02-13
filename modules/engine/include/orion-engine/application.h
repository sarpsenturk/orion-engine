#pragma once

namespace orion
{
    class Application
    {
    public:
        Application();
        Application(const Application&) = delete;
        Application(Application&&) = delete;
        Application& operator=(const Application&) = delete;
        Application& operator=(Application&&) = delete;
        virtual ~Application() = default;

        void on_update();
        void on_render();
        [[nodiscard]] bool should_exit() const noexcept;
        void run();

    private:
        virtual void on_user_update() = 0;
        virtual void on_user_render() = 0;
        [[nodiscard]] virtual bool user_should_exit() const noexcept = 0;
    };
} // namespace orion
