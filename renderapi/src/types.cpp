#include "orion-renderapi/types.h"

#include "orion-utils/assertion.h"
#include "orion-utils/hash.h"
#include "orion-utils/type.h"

#include <numeric>

template<>
struct std::hash<orion::DescriptorBinding> {
    std::size_t operator()(const orion::DescriptorBinding& binding) const
    {
        return 0ull |
               to_underlying(binding.type) |
               std::size_t{orion::to_underlying(binding.shader_stages)} << 8ull |
               std::size_t{binding.count} << 16ull;
    }
};

namespace orion
{
    std::string orion::format_as(ShaderStageFlags shader_stages)
    {
        if (!shader_stages) {
            return {};
        }

        std::string result{};
        for (auto stage : BitwiseRange{shader_stages}) {
            if (!stage) {
                continue;
            }
            switch (stage) {
                case ShaderStageFlags::Vertex:
                    result += "Vertex | ";
                    break;
                case ShaderStageFlags::Fragment:
                    result += "Fragment | ";
                    break;
            }
        }
        return result.substr(0, result.size() - 3);
    }

    DescriptorSetLayout::DescriptorSetLayout(std::initializer_list<DescriptorBinding> bindings)
        : bindings_(bindings)
        , hash_(hash_bindings(bindings))
    {
    }

    std::size_t DescriptorSetLayout::hash_bindings(std::span<const DescriptorBinding> bindings)
    {
        return std::accumulate(bindings.begin(), bindings.end(), bindings.size(), hash_combine<DescriptorBinding>);
    }

    bool DrawState::set_vertex_buffer(GPUBufferHandle new_vertex_buffer)
    {
        if (new_vertex_buffer.is_valid() && vertex_buffer != new_vertex_buffer) {
            vertex_buffer = new_vertex_buffer;
            return true;
        }
        return false;
    }

    bool DrawState::set_index_buffer(GPUBufferHandle new_index_buffer, IndexType new_index_type)
    {
        if (new_index_buffer.is_valid() && (index_buffer != new_index_buffer || index_type != new_index_type)) {
            index_buffer = new_index_buffer;
            index_type = new_index_type;
            return true;
        }
        return false;
    }

    bool DrawState::set_pipeline(PipelineHandle new_pipeline)
    {
        if (new_pipeline.is_valid() && (pipeline != new_pipeline)) {
            pipeline = new_pipeline;
            return true;
        }
        return false;
    }

    bool DrawState::set_viewport(const Viewport& new_viewport)
    {
        if (viewport != new_viewport) {
            viewport = new_viewport;
            return true;
        }
        return false;
    }

    bool DrawState::set_scissor(const Scissor& new_scissor)
    {
        if (scissor != new_scissor) {
            scissor = new_scissor;
            return true;
        }
        return false;
    }

    void DrawState::assert_valid_draw()
    {
        ORION_ASSERT(vertex_buffer.is_valid());
        ORION_ASSERT(pipeline.is_valid());
        ORION_ASSERT(viewport.size.sqr_magnitude() != 0);
        ORION_ASSERT(scissor.size.sqr_magnitude() != 0);
    }

    void DrawState::assert_valid_draw_indexed()
    {
        assert_valid_draw();
        ORION_ASSERT(index_buffer.is_valid());
        ORION_ASSERT(index_type != IndexType::None);
    }
} // namespace orion
