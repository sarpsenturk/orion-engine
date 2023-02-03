#pragma once

#include "application.h"

#include <memory> // std::unique_ptr

namespace orion
{
    class Engine final
    {
    public:
        explicit Engine(std::unique_ptr<Application> application);

        void main_loop();

    private:
        void update();
        void render();

        std::unique_ptr<Application> application_;
    };
} // namespace orion
