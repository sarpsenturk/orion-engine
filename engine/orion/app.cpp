#include "app.hpp"

namespace orion
{
    int App::run()
    {
        run_schedule(Schedule::Startup);
        run_schedule(Schedule::Shutdown);
        return 0;
    }

    void App::run_schedule(Schedule schedule)
    {
        for (auto& system : m_schedules[static_cast<std::size_t>(schedule)]) {
            system(m_world);
        }
    }
} // namespace orion
