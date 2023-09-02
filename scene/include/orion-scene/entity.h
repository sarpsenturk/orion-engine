#pragma once

#include "orion-utils/assertion.h"

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

namespace orion
{
    class Entity
    {
    public:
        Entity(entt::entity entity_id, entt::registry* registry);

        template<typename Component>
        [[nodiscard]] bool has_component() const
        {
            return registry_->any_of<Component>(entity_id_);
        }

        template<typename Component, typename... Args>
        decltype(auto) add_component(Args&&... args)
        {
            ORION_ASSERT(!has_component<Component>());
            return registry_->emplace<Component>(entity_id_, std::forward<Args>(args)...);
        }

        template<typename Component>
        void remove_component()
        {
            ORION_ASSERT(has_component<Component>());
            registry_->erase<Component>(entity_id_);
        }

        template<typename Component>
        [[nodiscard]] Component& get_component()
        {
            ORION_ASSERT(has_component<Component>());
            return registry_->get<Component>(entity_id_);
        }

        template<typename Component>
        const Component& get_component() const
        {
            ORION_ASSERT(has_component<Component>());
            return registry_->get<Component>(entity_id_);
        }

    private:
        entt::entity entity_id_;
        entt::registry* registry_;
    };
} // namespace orion
