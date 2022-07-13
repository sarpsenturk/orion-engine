#ifndef ORION_ENGINE_APPLICATION_H
#define ORION_ENGINE_APPLICATION_H

#include <arg-parse/arg-parse.h>

namespace orion
{
    class Application
    {
    public:
        virtual ~Application() = default;
        void on_create(const argparse::ArgParse& args);
        void on_shutdown();
        void run();

    private:
        virtual void on_user_create(const argparse::ArgParse& args) = 0;
        virtual void on_user_shutdown() = 0;
    };
} // namespace orion

#endif // ORION_ENGINE_APPLICATION_H
