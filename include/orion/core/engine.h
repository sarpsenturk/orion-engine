#ifndef ORION_ENGINE_ENGINE_H
#define ORION_ENGINE_ENGINE_H

#include "application.h"

namespace orion
{
    class Engine final
    {
    public:
        explicit Engine(Application::Ptr application, int argc,
                        const char* argv[]);
        [[nodiscard]] int main();

    private:
        [[nodiscard]] bool should_close() const noexcept;
        void on_update();
        void on_render();

    private:
        Application::Ptr application_;
    };
} // namespace orion

#endif // ORION_ENGINE_ENGINE_H
