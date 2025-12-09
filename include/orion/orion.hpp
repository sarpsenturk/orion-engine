#pragma once

#include "orion/application.hpp"

#include <memory>

namespace orion
{
    class Engine
    {
    public:
        Engine();

        void run(std::unique_ptr<Application> app);
    };
} // namespace orion
