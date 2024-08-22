# Orion Engine

Orion is a game engine, created with modern C++ (20) and best practices.

## Building

Orion will be initially only supported on Windows, however
Linux and macOS support will be added once the engine is in a more mature
state.

To build orion you will need:

- CMake 3.23 or higher
- Ninja
- C++ compiler with C++20 support
- Vulkan SDK (if building the Vulkan backend)
- Windows SDK (if building on Windows)

Orion uses vcpkg as a submodule to handle dependencies however
if you wish to install them some other way see [vcpkg.json](vcpkg.json)
for a full list of dependencies.

## Milestones

- [ ] Create a Window
- [ ] Render a triangle
- [ ] Render a quad
- [ ] Render a textured quad
- [ ] Render multiple quads
- [ ] Game: Pong
- [ ] Game: Breakout
