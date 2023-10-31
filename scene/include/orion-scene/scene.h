#pragma once

#include "entity.h"

#include "orion-math/matrix/matrix4.h"

#include <entt/entity/registry.hpp>

#include <string>
#include <vector>

namespace orion
{
    class Scene
    {
    public:
        Entity create_entity(std::string tag = "entity");

        template<typename... Components>
        [[nodiscard]] auto view() const
        {
            return registry_.view<Components...>();
        }

    private:
        entt::registry registry_;
    };
} // namespace orion
