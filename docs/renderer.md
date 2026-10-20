# Renderer

The guiding principle of the renderer is to provide a framework upon which applications can build their own rendering pipelines. It does not impose a specific rendering architecture or pipeline, but rather provides the tools and abstractions necessary to implement one along with some default implementations for common tasks. The renderer is designed to be flexible and extensible.

## Module overview

- `Renderer` owns the GPU device, main swapchain, per-frame command allocators and deletion queues, and render graph. It manages frame acquisition, execution, submission, and presentation without declaring rendering passes.
- `orion::gpu` namespace exposes a low-level, Vulkan-like RHI with opaque objects and explicit resource/command operations.
- `RenderGraph` records pass declarations and resource accesses, compiles barriers/attachments, and executes passes through the GPU API.

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
