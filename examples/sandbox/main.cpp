#include <orion/orion.hpp>

#include <cstdio>

using orion::App;

struct Sandbox {
    void build(App&)
    {
        std::puts("Hello from Sandbox::build()");
    }
};

int main()
{
    return App()
        .add_plugin(Sandbox{})
        .run();
}
