#pragma once

#include "app.hpp"
#include "logger.hpp"

namespace orion
{
    struct DefaultPlugins {
        LoggingPlugin logging = {};

        void build(App& app)
        {
            app
                .add_plugin(logging);
        }
    };
} // namespace orion
