#pragma once

#include <entt/entt.hpp>

#include <optional>

namespace orion
{
    class World
    {
    public:
        template<typename Resource>
        Resource& insert_resource(Resource&& resource)
        {
            return m_registry.ctx().insert_or_assign(std::forward<Resource>(resource));
        }

        template<typename Resource>
        Resource* get_resource()
        {
            return m_registry.ctx().find<Resource>();
        }

        template<typename Resource>
        const Resource* get_resource() const
        {
            return m_registry.ctx().find<Resource>();
        }

        template<typename Resource>
        std::optional<Resource> remove_resource()
        {
            std::optional<Resource> out;
            if (auto* resource = get_resource<Resource>()) {
                out.emplace(std::move(*resource));
                m_registry.ctx().erase<Resource>();
            }
            return out;
        }

    private:
        entt::registry m_registry;
    };
} // namespace orion
