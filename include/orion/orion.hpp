#pragma once

#include "orion/application.hpp"

#include <memory>

namespace orion
{
    class Engine
    {
    public:
        void run(std::unique_ptr<Application> app);
    };
} // namespace orion
