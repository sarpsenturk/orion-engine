#pragma once

namespace orion
{
    class Application
    {
    public:
        Application() = default;
        virtual ~Application() = default;

        void update(float dt);
        void render(float alpha);

        [[nodiscard]] bool should_close() const;

    protected:
        Application(const Application&) = default;
        Application& operator=(const Application&) = default;
        Application(Application&&) = default;
        Application& operator=(Application&&) = default;

    private:
        virtual void on_update(float dt) = 0;
        virtual void on_render(float alpha) = 0;
    };
} // namespace orion
