#include <orion/orion.hpp>

class SandboxApp : public orion::Application
{
public:
    SandboxApp(orion::Window* window)
        : window_(window)
    {
    }

private:
    void on_update() override {}
    void on_render() override {}
    bool should_exit() const override { return window_->should_close(); }

    orion::Window* window_;
};

int main()
{
    auto engine = orion::Engine::initialize();
    if (!engine) {
        return 1;
    } else {
        engine->run(std::make_unique<SandboxApp>(engine->window()));
        return 0;
    }
}
