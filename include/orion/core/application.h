#ifndef ORION_ENGINE_APPLICATION_H
#define ORION_ENGINE_APPLICATION_H

namespace orion
{
    class Application
    {
    public:
        virtual ~Application() = default;
        void on_create();
        void on_shutdown();
        void run();

    private:
        virtual void on_user_create() = 0;
        virtual void on_user_shutdown() = 0;
    };
} // namespace orion

#endif // ORION_ENGINE_APPLICATION_H
