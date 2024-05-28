#include "orion-core/window.h"

#include "orion-utils/bits.h"

#ifndef ORION_WINDOW_LOG_LEVEL
    #define ORION_WINDOW_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#include "orion-core/log.h"

#include <spdlog/spdlog.h>

namespace orion
{
    Window::Window(WindowDesc window_desc)
        : platform_window_(platform::create_window(window_desc), &platform::destroy_window)
        , name_(std::move(window_desc.name))
        , position_(window_desc.position)
        , size_(window_desc.size)
    {
    }

    WindowEvent Window::poll_event()
    {
        auto event = platform::poll_event_window(platform_window_.get());
        if (const auto* resize = event.get_if<WindowResize>()) {
            size_ = resize->size;
        } else if (const auto* move = event.get_if<WindowMove>()) {
            position_ = move->position;
        }
        return event;
    }

    spdlog::logger* Window::logger()
    {
        static const auto window_logger = create_logger("orion-window", ORION_WINDOW_LOG_LEVEL);
        return window_logger.get();
    }
} // namespace orion
