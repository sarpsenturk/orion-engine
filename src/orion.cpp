#include "orion/orion.hpp"

#include <cstdio>
#include <exception>

namespace orion
{
    void Engine::run(std::unique_ptr<Application> app)
    {
        try {
            while (!app->should_exit()) {
                app->update();
                app->render();
            }
        } catch (const std::exception& ex) {
            std::fputs(ex.what(), stderr);
        } catch (...) {
            std::fputs("Unknown exception", stderr);
        }
    }
} // namespace orion
