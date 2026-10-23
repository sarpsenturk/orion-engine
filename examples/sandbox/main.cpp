#include <orion/orion.hpp>

#include <orion/core/window.hpp>

class Sandbox : public orion::Application
{
    void on_update() override {}
    void on_render() override {}
    void on_event(const orion::Event&) override {}
    bool should_exit() const override { return m_window.should_close(); }

    orion::Window m_window{{"Sandbox", 1'280, 720}};
};

int main()
{
    auto engine = orion::Engine{};
    engine.run(std::make_unique<Sandbox>());
    return 0;
}
