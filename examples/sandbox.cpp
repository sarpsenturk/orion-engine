#include <orion/orion.hpp>

class Sandbox : public orion::Application
{
    void on_update() override {}
    void on_render() override {}
    bool on_should_close() const override { return true; }
};

int main()
{
    auto engine = orion::Engine();
    engine.run(std::make_unique<Sandbox>());
    return 0;
}