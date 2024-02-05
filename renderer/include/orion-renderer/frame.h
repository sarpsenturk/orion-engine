#pragma once

#include "orion-renderer/config.h"

#include "orion-utils/static_vector.h"

#include <algorithm>
#include <vector>

namespace orion
{
    [[nodiscard]] frame_index_t current_frame_index() noexcept;
    [[nodiscard]] frame_index_t previous_frame_index() noexcept;
    void advance_frame();

    template<typename Data>
    class PerFrameData
    {
    public:
        PerFrameData()
            requires(std::is_default_constructible_v<Data>)
            : frames_(frames_in_flight)
        {
        }

        template<typename G>
        explicit PerFrameData(G generator)
        {
            std::generate_n(std::back_inserter(frames_), frames_in_flight, generator);
        }

        [[nodiscard]] Data& current_frame() { return frames_[current_frame_index()]; }
        [[nodiscard]] const Data& current_frame() const { return frames_[current_frame_index()]; }

        [[nodiscard]] Data& previous_frame() { return frames_[previous_frame_index()]; }
        [[nodiscard]] const Data& previous_frame() const { return frames_[previous_frame_index()]; }

    private:
        static_vector<Data, frames_in_flight> frames_;
    };
} // namespace orion
