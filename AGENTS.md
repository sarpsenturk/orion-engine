# Project essentials

- Orion is a modern KSP successor: a physics sandbox with potentially interstellar scales. Game development has not started.
- `engine/` is the native C++ framework for the game and its tools; `game/` will contain the game. Let game requirements inform engine capabilities, but keep game concepts such as orbital mechanics out of engine code.
- Applications use the engine directly through C++; development is code-focused, without an editor or scripting boundary.
- Target Windows x64 with MSVC today; Linux and macOS are future targets. Desktop only; no consoles.
- Follow modern C++ best practices. Consider performance throughout development, while avoiding premature optimization. All code, interfaces, and architecture may change; there is no stable engine API.
- Follow `.clang-format` and the surrounding code. Engine compilation treats MSVC warnings as errors.

## Build, run, and test

Run from the repository root in an x64 Visual Studio developer environment. On the current Windows setup, tools such as CMake and clang-format are supplied by Visual Studio and are absent from the default shell. Initialize the developer shell first; see [development setup](docs/development.md#visual-studio-developer-shell).

```powershell
cmake --preset debug-x64-msvc-windows
cmake --build build/debug-x64-msvc-windows
ctest --test-dir build/debug-x64-msvc-windows --output-on-failure
./build/debug-x64-msvc-windows/examples/orion.sandbox.exe
```

Build and run relevant tests for code changes; use the sandbox for changes needing visual or interactive verification. Report any verification that could not be completed.

## Further context

Read the relevant document before working in its area, and update it when behavior or workflow changes:

- [Development](docs/development.md): prerequisites, shell setup, build options, and verification.
- [Architecture](docs/architecture.md): source map, application lifecycle, and engine/game responsibilities.
- [Renderer](docs/renderer.md): GPU backend, frame flow, render graph, shaders, and ImGui integration.
