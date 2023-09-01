#include "orion-scene/scene.h"

#include "orion-scene/components.h"

namespace orion
{
    Entity Scene::create_entity(std::string tag)
    {
        auto entity_id = registry_.create();
        registry_.emplace<TagComponent>(entity_id, std::move(tag));
        registry_.emplace<TransformComponent>(entity_id, Matrix4::identity());
        return {entity_id, &registry_};
    }
} // namespace orion
