#pragma once

#include "orion/application.hpp"

#include <memory>

namespace orion
{
    class Engine
    {
    public:
        Engine();
        ~Engine();

        void run(std::unique_ptr<Application> app);

    private:
        struct Window* window_;
    };
} // namespace orion
