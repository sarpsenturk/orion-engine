# Orion Engine

Orion is a high-performance, cross-platform game/game engine built from scratch
with modern C++, designed to create a one-to-one scale fully explorable universe.

## Getting Started

Orion is built using CMake. To get started, clone the repository and build the engine:
```bash
git clone https://github.com/orion-engine/orion.git
cd orion
cmake --preset <preset-name>
cmake --build build/<preset-name>
```

### Requirements

The project requires:
- C++23 compatible compiler (GCC, Clang, MSVC)
- CMake 3.30 or higher
- vcpkg

The presets make use of `vcpkg` for dependency management.
Make sure to install `vcpkg` and set it up according to the 
[official documentation](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started).

### Presets

Available presets can be listed using the following command:
```bash
cmake --list-presets
```

Presets follow the naming convention `<config>-<arch>-<compiler>-<platform>`, where:
- `<config>` is the build configuration (e.g., Debug, Release)
- `<arch>` is the target architecture (e.g., x64, ARM64)
- `<compiler>` is the compiler (e.g., gcc, clang, msvc)
- `<platform>` is the target platform (e.g., windows, linux, macos)

Refer to [CMakePresets.json](CMakePresets.json) file for the available presets.

## Project Structure

The project is organized as follows:
- `engine/`: Core engine code
- `examples/`: Example applications demonstrating engine features
- `game/`: Orion's game code

## Development

Orion is under active development and is in a very early stage.
All code is subject to change, and contributions are welcome.