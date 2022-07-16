#ifndef ORION_ENGINE_MOUSE_H
#define ORION_ENGINE_MOUSE_H

#include <orion-math/vector/vector2.h>

namespace orion
{
    class Mouse
    {
    public:
        void set_position(Vector2_i position) noexcept { position_ = position; }
        [[nodiscard]] auto& position() const noexcept { return position_; }

    private:
        Vector2_i position_;
    };
} // namespace orion

#endif // ORION_ENGINE_MOUSE_H
