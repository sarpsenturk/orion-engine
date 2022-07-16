#ifndef ORION_ENGINE_WINDOW_H
#define ORION_ENGINE_WINDOW_H

#include "orion/input/mouse.h"
#include "window_props.h"

#include <concepts>

namespace orion::detail
{
    template<typename WindowType>
    concept ValidWindow =
        requires(WindowType window, const WindowType const_window) {
            {
                const_window.props()
                } -> std::convertible_to<const WindowProps&>;
            {
                const_window.should_close()
                } -> std::convertible_to<bool>;
            {
                const_window.is_valid()
                } -> std::convertible_to<bool>;
            {
                window.initialize()
            };
            {
                window.destroy()
            };
            {
                window.update()
            };
            {
                const_window.mouse()
                } -> std::convertible_to<const Mouse&>;
        };

    template<ValidWindow WindowType>
    using WindowImpl = WindowType;
} // namespace orion::detail

#if defined(WIN32)
#include "orion/platform/win32/win_window.h"
namespace orion
{
    using Window = detail::WindowImpl<detail::WinWindow>;
}
#endif // Platform check

#endif // ORION_ENGINE_WINDOW_H
