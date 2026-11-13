# Orion Engine

Orion is a spiritual successor to Kerbal Space Program. It's being built from scratch with modern C++23 and Vulkan for performance and compatibility.

Orion is in early development. All code and APIs are subject to change.

## Building

```sh
cmake --preset <preset>
cmake --build build/<preset>
```

Choose a configure preset for your host platform:

| Preset | Platform | Compiler | Build type |
| --- | --- | --- | --- |
| `debug-x64-msvc-windows` | Windows | MSVC | `Debug` |
| `release-x64-msvc-windows` | Windows | MSVC | `RelWithDebInfo` |
| `debug-arm64-appleclang-macos` | macOS | AppleClang | `Debug` |
| `release-arm64-appleclang-macos` | macOS | AppleClang | `RelWithDebInfo` |

On Windows, run the commands from a Visual Studio developer environment so that `cl` is available.

### Requirements

- CMake 3.30+
- Ninja
- C++ compiler with support for C++23
- Vulkan SDK with Volk, Vulkan Memory Allocator (VMA), and the Slang compiler (`slangc`). Set up the SDK environment so CMake can find it and `slangc` is on `PATH`.
- [vcpkg](https://github.com/microsoft/vcpkg), with `VCPKG_ROOT` set to its installation directory, is required by the presets. See [vcpkg.json](vcpkg.json) for the managed dependencies. To use another dependency setup, configure without these presets and provide the packages required by [CMakeLists.txt](CMakeLists.txt).

### Build options

| Option | Description | Default |
| --- | --- | --- |
| `ORION_BUILD_EXAMPLES` | Build the example applications. | `ON` when configured as the top-level project; otherwise `OFF`. |
| `ORION_BUILD_TESTS` | Build the tests. | `ON` when configured as the top-level project; otherwise `OFF`. |

Pass options during configuration, for example:

```sh
cmake --preset <preset> -DORION_BUILD_EXAMPLES=OFF -DORION_BUILD_TESTS=OFF
```

## Running tests

With `ORION_BUILD_TESTS` enabled, build the project and run:

```sh
ctest --test-dir build/<preset> --output-on-failure
```
