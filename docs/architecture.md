# Architecture

## Purpose and boundaries

Orion is a modern, feature-rich successor to KSP: a physics sandbox with potentially interstellar scales. The engine/framework exists to build that game and its surrounding tools. Game development has not started.

Game requirements should shape the engine's capabilities. Domain concepts such as orbital mechanics belong in the future `game/` layer; reusable engine mechanisms belong in `engine/`. The engine must not depend on the game. Large-scale requirements may affect engine design without making the engine responsible for the game's simulation rules.

The game and tools will be native C++ applications using the engine directly. The development model is code-focused, without an editor or scripting boundary. There is no stable engine API: implementation, interfaces, and architecture are all open to change.

## Source map

| Location | Current responsibility |
| --- | --- |
| `engine/orion/orion.hpp`, `orion.cpp` | `Engine`, `Application`, service ownership, and main loop. |
| `engine/orion/core/` | Logging, assertions, platform initialization, windows, and input. The platform implementation uses GLFW. |
| `engine/orion/renderer/` | Renderer, camera data, GPU API/backend, and render graph; see [renderer](renderer.md). |
| `engine/orion/controllers/` | Fly-camera input controller. |
| `engine/orion/lib/` | Small utilities such as hashing, scope guards, slot maps, and inline callable storage. |
| `engine/orion/memory/` | Initial memory alignment helpers. |
| `engine/shaders/` | Slang shader sources and offline compilation rules. |
| `engine/imgui/` | Vendored Dear ImGui, its backends, and a local backend patch under `patches/`. |
| `engine/tests/` | Engine unit tests. |
| `examples/sandbox/` | Native example application and interactive development harness. |
| `cmake/` | Shared build helpers, currently GoogleTest integration. |
| `game/` (planned) | Orion's game and simulation code; directory not yet present. |

`orion.engine` is the library target. `orion.sandbox` links to it; the example in [main.cpp](../examples/sandbox/main.cpp) is the shortest reference for application integration. See [development](development.md) for build and test details.

## Current application lifecycle

`Engine` owns the logger, platform, window, input, and renderer. Construction initializes those services in that order; destruction follows reverse member order, keeping the window/platform alive through renderer teardown.

An application derives from `Application`, implements `on_update(float dt)` and `on_render()`, and is passed to `Engine::run` as a `std::unique_ptr`. The sandbox retains references to the engine services it uses; the engine must outlive those references.

Each main-loop iteration:

1. Poll platform events.
2. Compute elapsed time in seconds and call the application's update callback.
3. Attempt to begin a rendering frame. On success, call the application's render callback, then end/present the frame. The sandbox's render callback builds ImGui UI and calls `Renderer::render`.
4. If frame acquisition fails, attempt swapchain resizing when the framebuffer dimensions are nonzero.
5. Update and render additional ImGui platform windows when multi-viewport support is enabled.

Updates currently use variable elapsed time; there is no game simulation or fixed-step simulation loop yet. This describes the current implementation, not a commitment for the future game's physics loop.

## Documentation scope

Keep repository-wide instructions in [AGENTS.md](../AGENTS.md), setup and verification in [development](development.md), and module details in focused documents. Update these documents alongside changes to the behavior they describe.

For now, the renderer shares one document covering its closely related components. Split out GPU or render-graph documentation when either needs substantial independent guidance. Add game, simulation, asset, or tool documents when those systems exist or their design is being actively decided; avoid speculative feature documentation.
