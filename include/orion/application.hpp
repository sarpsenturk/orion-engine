#pragma once

namespace orion
{
    class Application
    {
    public:
        Application() = default;
        virtual ~Application() = default;

        void update();
        void render();

        [[nodiscard]] virtual bool should_exit() const = 0;

    protected:
        Application(const Application&) = default;
        Application& operator=(const Application&) = default;
        Application(Application&&) = default;
        Application& operator=(Application&&) = default;

    private:
        virtual void on_update() = 0;
        virtual void on_render() = 0;
    };
} // namespace orion
