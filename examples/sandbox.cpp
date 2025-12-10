#include <orion/orion.hpp>

class Sandbox : public orion::Application
{
    void on_update(float /*dt*/) override {}
    void on_render(float /*alpha*/) override {}
};

int main()
{
    auto engine = orion::Engine();
    engine.run(std::make_unique<Sandbox>());
    return 0;
}