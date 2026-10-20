# Renderer

The guiding principle of the renderer is to provide a framework upon which applications can build their own rendering pipelines. It does not impose a specific rendering architecture or pipeline, but rather provides the tools and abstractions necessary to implement one along with some default implementations for common tasks. The renderer is designed to be flexible and extensible.

## Module overview

- `Renderer` owns the GPU device, main swapchain, per-frame command allocators and deletion queues, and render graph. It manages frame acquisition, execution, submission, and presentation without declaring rendering passes.
- `orion::gpu` namespace exposes a low-level, Vulkan-like RHI with opaque objects and explicit resource/command operations.
- `RenderGraph` records pass declarations and resource accesses, compiles barriers/attachments, and executes passes through the GPU API.

## Geometry

`Renderer::geometry_manager()` owns mesh storage: a 16 MiB positions buffer and a 4 MiB shared `uint32_t` index buffer, each suballocated with its own `OffsetAllocator`. Both use `gpu::Memory::Default`, currently assumed host visible. Mesh uploads are copied during `create_mesh`; the source spans need not outlive the call. Positions must be nonempty and match `MeshDesc::vertex_count`; indices may be empty for non-indexed draws.

`get_geometry(mesh)` returns the mesh's pre-offset position address and draw counts. Pass that address to the shader, bind `index_buffer()`, and use `first_index` with `gpu::draw_indexed`. Indices are local to the mesh, so `vertex_offset` stays zero. Non-indexed draws likewise start at vertex zero.

`destroy_mesh` immediately invalidates the handle. Renderer queues allocation reclamation after its next graphics submission and runs it when that frame completes. Until then, the mesh still consumes buffer capacity and counts toward the 65,536 allocation limit; exhaustion asserts. This lifetime tracking covers renderer frame submissions; work on other queues must finish or be joined into the frame submission before reclamation. At shutdown Renderer waits for the GPU and flushes frame callbacks while its value-owned geometry manager is alive. The geometry buffers and remaining renderer resources are then destroyed before the device, which owns the final lifetime scope.

## Render graph

All code lives under `engine/orion/renderer/render_graph/`:

- `graph.*`: pass declarations, compilation, and execution.
- `builder.*`: pass setup and declared accesses; currently exposes color attachments.
- `context.*`: execution-time access to imported GPU resources and the command buffer.
- `resource.*`: typed resource handles, descriptions, and access/layout states.

The structure of defining a render pass is as follows:
```cpp
graph.add_pass(
    "ImGui", RenderPassType::Graphics,
    [&](RenderPassBuilder& builder) {
        return builder.add_color_attachment(target, load_op, store_op, clear_color);
    },
    [](RenderPassContext& context, RGTexture /*target*/) {
        ImGui_ImplOrion_RenderDrawData(ImGui::GetDrawData(), context.command_buffer());
    });
```

Two callables are `setup` & `execute`:

- `setup`: Invoked immediately, it declares pass resources & usages, returns pass outputs.
- `execute`: Stored until graph execution, it accepts whatever was returned from `setup` to invoke rendering commands.
