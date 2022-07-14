#ifndef ORION_ENGINE_APPLICATION_H
#define ORION_ENGINE_APPLICATION_H

#include <arg-parse/arg-parse.h>
#include <memory>

namespace orion
{
    class Application
    {
    public:
        using Ptr = std::unique_ptr<Application>;

    public:
        virtual ~Application() = default;
        void on_create(const argparse::ArgParse& args);
        void on_shutdown();
        void on_update();
        [[nodiscard]] bool should_close() const noexcept;

    private:
        virtual void on_user_create(const argparse::ArgParse& args) = 0;
        virtual void on_user_shutdown() = 0;
        virtual void on_user_update() = 0;
        [[nodiscard]] virtual bool user_should_close() const noexcept = 0;
    };
} // namespace orion

#endif // ORION_ENGINE_APPLICATION_H
