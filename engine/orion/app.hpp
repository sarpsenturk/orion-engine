#pragma once

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

        // Execution
        int run();

    private:
        entt::registry m_registry;
        entt::dense_set<entt::id_type> m_plugins;
    };
} // namespace orion
