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

        [[nodiscard]] bool should_close() const;

    protected:
        Application(const Application&) = default;
        Application& operator=(const Application&) = default;
        Application(Application&&) = default;
        Application& operator=(Application&&) = default;

    private:
        virtual void on_update() = 0;
        virtual void on_render() = 0;

        [[nodiscard]] virtual bool on_should_close() const = 0;
    };
} // namespace orion
