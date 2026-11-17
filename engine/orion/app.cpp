#include "app.hpp"

namespace orion
{
    int App::run()
    {
        run_schedule(Schedule::Startup);
        m_runner(*this);
        run_schedule(Schedule::Shutdown);
        return 0;
    }

    void App::run_schedule(Schedule schedule)
    {
        for (auto& system : m_schedules[static_cast<std::size_t>(schedule)]) {
            system(m_world);
        }
    }

    void App::do_frame()
    {
        run_schedule(Schedule::Update);
    }

    App& App::set_runner(Runner runner)
    {
        m_runner = std::move(runner);
        return *this;
    }
} // namespace orion
