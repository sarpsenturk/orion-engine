# Orion Engine
The Orion Engine is a modern, cross-platform game engine written in C++ 20.
It is designed with to be fast and easy to use.

## Features
* Windowing system
* Graphics API abstraction layer
* Cross-API rendering support (Vulkan, D3D12, D3D11, OpenGL, Metal)
* Lots more to come!

## Building
As Orion supports multiple operating systems, build requirements may vary across
operating systems.
### Requirements
* [CMake >= 3.21](https://cmake.org/download/)
* [Ninja](https://ninja-build.org/)
* [vcpkg](https://github.com/microsoft/vcpkg)
* C++ compiler with C++ 20 support
* [DirectX Shader Compiler](https://github.com/microsoft/DirectXShaderCompiler/) ( >= v1.7.2212 for SPIRV generation)

Once all requirements are installed:

```
git clone --recurse-submodules https://github.com/sarpsenturk/orion-engine
cd orion-engine
mkdir build && cd build
cmake -DDXC_PATH=<dxc-base-path> --preset=<selected-preset> ..
```
