#pragma once

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

    protected:
        Application(const Application&) = default;
        Application(Application&&) noexcept = default;
        Application& operator=(const Application&) = default;
        Application& operator=(Application&&) noexcept = default;

    private:
        virtual void on_user_update() = 0;
        virtual void on_user_render() = 0;
        [[nodiscard]] virtual bool user_should_exit() const noexcept = 0;
    };
} // namespace orion
