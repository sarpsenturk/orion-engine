#pragma once

#include "builder.hpp"
#include "context.hpp"
#include "resource.hpp"

#include "orion/lib/inline_function.hpp"
#include "orion/renderer/gpu/gpu.hpp"

#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace orion
{
    class RenderGraph
    {
    public:
        static constexpr auto max_pass_callback_size = 128;

        template<
            typename SetupFn,
            typename ExecuteFn,
            typename Params = std::invoke_result_t<SetupFn, RenderPassBuilder&>>
            requires(std::is_invocable_v<SetupFn, RenderPassBuilder&> &&
                     std::is_invocable_v<ExecuteFn, RenderPassContext&, Params> &&
                     std::is_trivially_copyable_v<Params>)
        Params add_pass(std::string name, SetupFn&& setup, ExecuteFn&& execute)
        {
            const auto index = static_cast<std::uint32_t>(m_passes.size());
            auto& pass       = m_passes.emplace_back(std::move(name));
            auto builder     = RenderPassBuilder(this, index);
            auto params      = setup(builder);
            pass.execute     = [execute = std::forward<ExecuteFn>(execute),
                                params  = params](RenderPassContext& context) {
                execute(context, params);
            };
            return params;
        }

        void compile();

        void execute(gpu::CommandBuffer* command_buffer);

        void reset();

        RGTexture import_texture(std::string name, const RGTextureDesc& desc, const RGTextureImportDesc& import);
        RGBuffer import_buffer(std::string name, const RGBufferDesc& desc, const RGBufferImportDesc& import);

    private:
        friend class RenderPassBuilder;
        friend class RenderPassContext;

        struct ResourceVersion {
            std::vector<std::uint32_t> reader_passes;
            std::uint32_t writer_pass = ~0u;
            bool contents_defined     = false;
        };

        using ResourceDesc       = std::variant<RGTextureDesc, RGBufferDesc>;
        using ResourceImportDesc = std::variant<RGTextureImportDesc, RGBufferImportDesc>;

        struct Resource {
            std::string name;
            ResourceDesc desc;
            std::optional<ResourceImportDesc> import_desc;
            std::vector<ResourceVersion> versions;

            ResourceVersion& add_version() { return versions.emplace_back(); }
            [[nodiscard]] std::uint32_t latest_version_number() const { return static_cast<std::uint32_t>(versions.size() - 1); }
            [[nodiscard]] const ResourceVersion& latest_version() const { return versions.back(); }
        };

        struct GpuTexture {
            gpu::Image* image    = nullptr;
            gpu::ImageView* view = nullptr;
        };

        struct GpuBuffer {
            gpu::Buffer* buffer = nullptr;
        };

        using GpuResource = std::variant<GpuTexture, GpuBuffer>;

        struct ResourceAccess {
            std::uint32_t resource_index = ~0u;
            RGResourceState state;
        };

        struct AttachmentRef {
            std::uint32_t resource_index = ~0u;
            gpu::LoadOp load_op          = gpu::LoadOp::Clear;
            gpu::StoreOp store_op        = gpu::StoreOp::Store;
            gpu::ClearValue clear_value  = {};
        };

        struct RenderPass {
            // Debug name for the render pass
            std::string name;

            // Callback to execute the render pass, with the parameters returned from the setup function
            InlineFunction<max_pass_callback_size, void(RenderPassContext&)> execute;

            // All resources used in this render pass, along with their access states
            std::vector<ResourceAccess> resource_accesses;

            // Color attachments for the render pass
            std::vector<AttachmentRef> color_attachments;
            // Depth attachment for the render pass
            std::optional<AttachmentRef> depth_attachment;

            // Compiled barriers for the render pass
            std::vector<gpu::ImageBarrier> image_barriers;
            std::vector<gpu::BufferBarrier> buffer_barriers;
        };

        [[nodiscard]] gpu::ImageBarrier make_image_barrier(
            gpu::Image* image,
            const RGResourceState& before,
            const RGResourceState& after) const;
        [[nodiscard]] gpu::BufferBarrier make_buffer_barrier(
            gpu::Buffer* buffer,
            const RGResourceState& before,
            const RGResourceState& after) const;

        std::vector<RenderPass> m_passes;

        std::vector<Resource> m_resources;
        std::flat_map<std::uint32_t, GpuResource> m_gpu_resources;

        std::vector<gpu::ImageBarrier> m_final_image_barriers;
        std::vector<gpu::BufferBarrier> m_final_buffer_barriers;
    };
} // namespace orion
