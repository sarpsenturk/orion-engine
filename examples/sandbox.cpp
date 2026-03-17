#include <orion/orion.hpp>

class SandboxApp : public orion::Application
{
    void on_update() override {}
    void on_render() override {}
    bool should_exit() const override { return false; }
};

int main()
{
    auto engine = orion::Engine{};
    engine.run(std::make_unique<SandboxApp>());
}
