#ifndef ORION_ENGINE_MOUSE_EVENT_H
#define ORION_ENGINE_MOUSE_EVENT_H

#include "event_dispatcher.h"
#include "event_handler.h"

#include <orion-math/vector/vector2.h>

namespace orion
{
    struct MouseMoveEvent {
        Vector2_i position;
    };

    class MouseEventHandler : public EventHandler<MouseMoveEvent>
    {
    };

    class MouseEventDispatcher final : public EventDispatcher<MouseMoveEvent>
    {
    };
} // namespace orion

#endif // ORION_ENGINE_MOUSE_EVENT_H
