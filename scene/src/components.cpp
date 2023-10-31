#include "orion-scene/components.h"

#include "orion-math/matrix/transformation.h"

namespace orion
{
    void TransformComponent::translate(const Vector3_f& translation)
    {
        transform = transform * orion::translation(translation);
    }
} // namespace orion
