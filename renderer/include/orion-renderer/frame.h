#pragma once

#include "orion-renderer/config.h"

#include "orion-utils/static_vector.h"

#include <algorithm>
#include <vector>

namespace orion
{
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

        [[nodiscard]] Data& get(frame_index_t index) { return frames_[index]; }
        [[nodiscard]] const Data& get(frame_index_t index) const { return frames_[index]; }

    private:
        static_vector<Data, frames_in_flight> frames_;
    };
} // namespace orion
