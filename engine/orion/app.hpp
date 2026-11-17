#pragma once

#include "world.hpp"

#include <entt/entt.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <vector>

namespace orion
{
    using System = std::function<void(World&)>;

    class App
    {
    public:
        enum class Schedule {
            Startup,
            Shutdown,

            COUNT,
        };

        App() = default;

        // Plugin system
        template<typename Plugin>
        App& add_plugin(Plugin plugin)
        {
            const auto type_index = entt::type_index<Plugin>::value();
            if (m_plugins.insert(type_index).second) {
                plugin.build(*this);
            }
            return *this;
        }

        // Systems and schedules
        App& add_system(Schedule schedule, System system)
        {
            m_schedules[static_cast<std::size_t>(schedule)]
                .push_back(std::move(system));
            return *this;
        }

        // World context & resources
        template<typename Resource>
        App& insert_resource(Resource&& resource)
        {
            m_world.insert_resource(std::forward<Resource>(resource));
            return *this;
        }

        [[nodiscard]] World& world() { return m_world; }
        [[nodiscard]] const World& world() const { return m_world; }

        // Execution
        int run();
        void run_schedule(Schedule schedule);

    private:
        World m_world;
        entt::dense_set<entt::id_type> m_plugins;
        std::array<std::vector<System>, static_cast<std::size_t>(Schedule::COUNT)> m_schedules;
    };
} // namespace orion
