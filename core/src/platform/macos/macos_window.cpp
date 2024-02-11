#include "orion-core/window.h"

#include "orion-utils/assertion.h"

namespace orion
{
    namespace platform
    {
        PlatformWindow* create_window(Window* this_ptr, const WindowDesc& window_desc)
        {
            ORION_ASSERT(!"Not implemented");
            return nullptr;
        }

        void destroy_window(PlatformWindow* platform_window)
        {
            if (platform_window != nullptr) {
                ORION_ASSERT(!"Not implemented");
            }
        }

        void update_window(PlatformWindow* platform_window)
        {
            ORION_ASSERT(!"Not implemented");
        }
    } // namespace platform
} // namespace orion