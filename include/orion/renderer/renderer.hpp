#pragma once

namespace orion
{
    class Renderer
    {
    public:
        static bool init(const struct Window* window);
        static void shutdown();

        static void render();
    };
} // namespace orion
