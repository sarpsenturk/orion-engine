#pragma once

#include "world.hpp"

#include <entt/entt.hpp>

namespace orion
{
    class App
    {
    public:
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

    private:
        World m_world;
        entt::dense_set<entt::id_type> m_plugins;
    };
} // namespace orion
