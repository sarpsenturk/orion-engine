#pragma once

#include "orion/application.hpp"
#include "orion/log.hpp"

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
    };
} // namespace orion
