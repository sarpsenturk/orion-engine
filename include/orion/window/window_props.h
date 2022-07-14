#ifndef ORION_ENGINE_WINDOW_PROPS_H
#define ORION_ENGINE_WINDOW_PROPS_H

#include <orion-math/vector/vector2.h>
#include <string>

namespace orion
{
    struct WindowProps {
        static constexpr int kUseDefault = -1;
        static constexpr Vector2_i kDefaultSize{kUseDefault, kUseDefault};
        static constexpr Vector2_i kDefaultPosition{kUseDefault, kUseDefault};

        std::string name = "Orion Engine";
        Vector2_i size = kDefaultSize;
        Vector2_i position = kDefaultPosition;
    };
} // namespace orion

#endif // ORION_ENGINE_WINDOW_PROPS_H
