#ifndef ORION_ENGINE_WINDOW_EVENT_H
#define ORION_ENGINE_WINDOW_EVENT_H

#include "event_dispatcher.h"
#include "event_handler.h"

#include <orion-math/vector/vector2.h>
#include <string_view>

namespace orion
{
    struct WindowCreateEvent {
        std::string_view window_name;
    };

    struct WindowCloseEvent {
        std::string_view window_name;
    };

    struct WindowMoveEvent {
        std::string_view window_name;
        Vector2_i position;
    };

    struct WindowResizeEvent {
        std::string_view window_name;
        Vector2_i size;
    };

    class WindowEventHandler
        : public EventHandler<WindowCreateEvent>
        , public EventHandler<WindowCloseEvent>
        , public EventHandler<WindowMoveEvent>
        , public EventHandler<WindowResizeEvent>
    {
    };

    class WindowEventDispatcher
        : public EventDispatcher<WindowCreateEvent, WindowCloseEvent,
                                 WindowMoveEvent, WindowResizeEvent>
    {
    };
} // namespace orion

#endif // ORION_ENGINE_WINDOW_EVENT_H
