#include <orion/orion.hpp>

#include <cstdio>

using orion::App;
using orion::DefaultPlugins;
using orion::World;

struct Sandbox {
    struct Counter {
        int value = 0;
    };

    static void on_startup(World& world)
    {
        orion::log::add_channel("sandbox");
        ORION_LOG_INFO("sandbox", "Sandbox::on_startup({})", "trace");
    }

    static void on_shutdown(World& world)
    {
        ORION_LOG_INFO("sandbox", "Sandbox::on_shutdown()");
    }

    void build(App& app)
    {
        app
            .add_system(App::Schedule::Startup, on_startup)
            .add_system(App::Schedule::Shutdown, on_shutdown);
    }
};

int main()
{
    return App()
        .add_plugin(DefaultPlugins{})
        .add_plugin(Sandbox{})
        .run();
}
