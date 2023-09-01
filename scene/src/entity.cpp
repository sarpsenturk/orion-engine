#include "orion-scene/entity.h"

namespace orion
{
    Entity::Entity(entt::entity entity_id, entt::registry* registry)
        : entity_id_(entity_id)
        , registry_(registry)
    {
    }
} // namespace orion
