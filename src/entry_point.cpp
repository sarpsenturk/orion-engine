#include "orion/application.h"

// Temporary entry point definition
// This will change as different entry points are needed
// for different platforms
int main(int argc, const char* argv[])
{
    try {
        auto app = orion_main(std::span{argv, static_cast<std::size_t>(argc)});
        while (!app->should_exit()) {
            app->update();
            app->render();
        }
        return 0;
    } catch (const std::exception& err) {
        std::fputs("Error: ", stderr);
        std::fputs(err.what(), stderr);
        return 1;
    } catch (...) {
        std::fputs("unknown exception", stderr);
        return 1;
    }
}
