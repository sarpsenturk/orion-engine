#include "orion-renderer/frame.h"

namespace orion
{
    // Yes globals. Not nice but will do.
    namespace
    {
        frame_index_t current_frame_index_ = 0;
        frame_index_t previous_frame_index_ = -1;
    } // namespace
} // namespace orion
