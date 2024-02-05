#include "orion-renderer/frame.h"

namespace orion
{
    // Yes globals. Not nice but will do.
    namespace
    {
        frame_index_t current_frame_index_ = 0;
        frame_index_t previous_frame_index_ = -1;
    } // namespace

    frame_index_t current_frame_index() noexcept
    {
        return current_frame_index_;
    }

    frame_index_t previous_frame_index() noexcept
    {
        return previous_frame_index_;
    }

    void advance_frame()
    {
        previous_frame_index_ = current_frame_index_;
        current_frame_index_ = (current_frame_index_ + frame_index_t{1}) % frames_in_flight;
    }
} // namespace orion
