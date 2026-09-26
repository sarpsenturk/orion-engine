#include <orion/orion.hpp>

class SandboxApp : public orion::Application
{
    void on_update(float dt) override {}
    void on_render() override {}
};

int main()
{
    auto engine = orion::Engine{};
    engine.run(std::make_unique<SandboxApp>());
    return 0;
}
