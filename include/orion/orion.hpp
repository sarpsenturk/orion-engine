#pragma once

#include "orion/application.hpp"
#include "orion/log.hpp"
#include "orion/window.hpp"

#include <memory>

namespace orion
{
    class Engine
    {
    public:
        Engine();

        void run(std::unique_ptr<Application> app);

    private:
        std::shared_ptr<Logger> logger_;
        std::unique_ptr<Window> window_;
    };
} // namespace orion
