#pragma once

#include <glm/mat4x4.hpp>

namespace orion
{
    struct Camera {
        glm::mat4 view       = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
    };
} // namespace orion
