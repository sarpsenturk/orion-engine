#pragma once

#include "orion/core/assert.hpp"
#include "orion/gpu/gpu.hpp"

#include <array>
#include <cstddef>
#include <cstring>
#include <span>
#include <vector>

namespace orion
{
    struct DrawPacket {
        static constexpr std::size_t max_root_size = 128z;

        gpu::PipelineState* pipeline    = nullptr;
        gpu::Buffer* index_buffer       = nullptr;
        std::size_t index_buffer_offset = 0z;
        gpu::DrawIndexed draw;
        std::array<std::byte, max_root_size> root_data = {};
        std::size_t root_size = 0z;

        void set_root(std::span<const std::byte> root) noexcept
        {
            ORION_ASSERT(root.size() <= max_root_size);
            ORION_ASSERT(root.size() % 4z == 0z);
            if (!root.empty()) {
                std::memmove(root_data.data(), root.data(), root.size());
            }
            root_size = root.size();
        }

        [[nodiscard]] gpu::ByteSpan root() const noexcept
        {
            return {root_data.data(), root_size};
        }

    };

    using OpaqueDrawList = std::vector<DrawPacket>;
} // namespace orion
