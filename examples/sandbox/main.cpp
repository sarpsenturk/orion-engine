#include <orion/orion.hpp>

#include <cstdio>

using orion::App;
using orion::World;

struct Sandbox {
    struct Counter {
        int value = 0;
    };

    static void on_startup(World& world)
    {
        std::puts("Sandbox::on_startup()");
    }

    static void on_shutdown(World& world)
    {
        std::puts("Sandbox::on_shutdown()");
    }

    void build(App& app)
    {
        std::puts("Hello from Sandbox::build()");
        app
            .add_system(App::Schedule::Startup, on_startup)
            .add_system(App::Schedule::Shutdown, on_shutdown);
    }
};

int main()
{
    return App()
        .add_plugin(Sandbox{})
        .run();
}
