#pragma once

#include "app.hpp"
#include "core/logger.hpp"
#include "core/window.hpp"

namespace orion
{
    struct DefaultPlugins {
        LoggingPlugin logging = {};
        WindowPlugin window   = {};

        void build(App& app)
        {
            app
                .add_plugin(logging)
                .add_plugin(window);
        }
    };
} // namespace orion
