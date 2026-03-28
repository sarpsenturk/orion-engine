#pragma once

#include "orion/application.hpp"
#include "orion/log.hpp"
#include "orion/window.hpp"

#include <tl/expected.hpp>

#include <memory>
#include <string>

namespace orion
{
    class Engine
    {
    public:
        static tl::expected<Engine, std::string> initialize();

        void run(std::unique_ptr<Application> app);

        [[nodiscard]] Window* window() { return &window_; }
        [[nodiscard]] const Window* window() const { return &window_; }

    private:
        Engine(Logger logger, Window window);
        Logger logger_;
        Window window_;
    };
} // namespace orion
