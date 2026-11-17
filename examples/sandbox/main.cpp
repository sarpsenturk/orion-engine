#include <orion/orion.hpp>

#include <cstdio>

using orion::App;

struct Sandbox {
    struct Counter {
        int value = 0;
    };

    void build(App& app)
    {
        std::puts("Hello from Sandbox::build()");
        app.insert_resource(Counter{});
        app.world().get_resource<Counter>()->value++;
        std::printf("Counter: %d\n", app.world().get_resource<Counter>()->value);
    }
};

int main()
{
    return App()
        .add_plugin(Sandbox{})
        .run();
}
