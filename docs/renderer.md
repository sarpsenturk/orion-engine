# Renderer

This describes the current implementation, which is early and subject to change. Start at [renderer.cpp](../engine/orion/renderer/renderer.cpp) for the frame path and [the sandbox](../examples/sandbox/main.cpp) for application usage.

## Responsibilities

- `Renderer` owns the GPU device, main swapchain, per-frame command allocators and deletion queues, render graph, and ImGui context/integration. It also currently allocates a depth image/view that the active passes do not use.
- `orion::gpu` exposes opaque GPU objects and explicit resource/command operations in [gpu.hpp](../engine/orion/renderer/gpu/gpu.hpp). The sole backend is [gpu_impl_vulkan.cpp](../engine/orion/renderer/gpu/gpu_impl_vulkan.cpp), which uses Vulkan, Volk, and Vulkan Memory Allocator.
- `RenderGraph` records pass declarations and resource accesses, compiles barriers/attachments, and executes passes through the GPU API.

The backend requests Vulkan 1.3, dynamic rendering, synchronization2, timeline semaphores, buffer device addresses, and descriptor-indexing features. Read device creation for the exact enabled feature set when changing requirements. Graphics, compute, and transfer queues exist in the GPU API, but the current renderer submits its main frame on the graphics queue.

## Render graph

Source is under `engine/orion/renderer/render_graph/`:

- `graph.*`: pass declarations, compilation, and execution.
- `builder.*`: pass setup and declared accesses; currently exposes color attachments.
- `context.*`: execution-time access to imported GPU resources and the command buffer.
- `resource.*`: typed resource handles, descriptions, and access/layout states.

`add_pass` runs setup immediately. Its returned parameters must be trivially copyable and are captured with the execute callback for later execution. Execute callbacks use fixed inline storage, so their captures must fit `max_pass_callback_size` and satisfy `InlineFunction`'s constraints. Keep deferred captures alive until execution; returning resource handles as pass parameters is the existing pattern.

Resource handles contain an index and version. Writing to a resource creates a new version and returns its handle.

The current graph imports externally owned textures/buffers and executes graphics passes in declaration order. Compilation resolves attachments, tracks resource states, and emits barriers for hazards/layout transitions and imported resources' final states. Version bookkeeping does not currently schedule or cull passes. There is no public transient-resource allocation API, compute-pass support, or general shader-read/write declaration API yet.

Reset before declaring each frame's passes, compile once, and execute while imported objects remain valid. The current renderer follows that sequence.

## Resource lifetime and synchronization

GPU resources use explicit create/destroy operations. Imported graph resources remain owned by the caller; resetting the graph clears its bookkeeping without destroying those resources.

Resources, command allocators, and command data must remain valid until the GPU has finished using them. Frame-slot reuse is guarded by a timeline wait. A `DeletionQueue` defers destruction but performs no synchronization itself: flush only after the relevant GPU work completes, and enqueue dependent objects in a safe destruction order. The renderer flushes frame queues after waiting and waits idle during teardown.

Changes to submission, presentation, resizing, or deferred deletion must preserve both GPU completion and resource-state requirements. CPU scope alone does not determine when a GPU object can be destroyed.

## Shaders and ImGui

[engine/shaders/CMakeLists.txt](../engine/shaders/CMakeLists.txt) registers shader programs with `orion_add_shader`. Slang compiles them offline to SPIR-V using the `spirv_1_6` profile, with dependency files for incremental builds. Output defaults to `<build-dir>/engine/shaders`; `ORION_SHADER_OUTPUT_DIR` overrides it. Building `orion.engine` builds `orion.shaders`. The current triangle and mesh programs are compiled but are not drawn by the active clear/ImGui path.

Dear ImGui is vendored under `engine/imgui/` with docking and multi-viewport support enabled by `Renderer`. The `ImGui_ImplOrion_*` bridge lives at the end of the Vulkan backend and connects ImGui's GLFW/Vulkan backends to engine GPU objects. The Vulkan backend has a local transition fix recorded under `engine/imgui/patches/`; review that patch when updating the vendored backend.

For verification, use the sandbox checks in [development](development.md#run-and-verify). The existing utility unit tests do not cover GPU rendering or render-graph behavior.
