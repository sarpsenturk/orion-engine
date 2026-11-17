#pragma once

#include <string>

namespace orion
{
    struct Window {
        void* handle;
        int width;
        int height;
    };

    struct WindowPlugin {
        struct Config {
            std::string title = "Orion";
            int width         = 1'280;
            int height        = 720;
        } config = {};

        void build(class App& app);
    };
} // namespace orion
