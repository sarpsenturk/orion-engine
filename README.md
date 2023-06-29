# Orion Engine
The Orion Engine is a modern, cross-platform game engine written in C++ 20.
It is designed with to be fast and easy to use.

## Features
* Windowing system
* Graphics API abstraction layer
* Lots more to come!

## Building
As Orion supports multiple operating systems, build requirements may vary across
operating systems.
### Requirements
* [CMake >= 3.21](https://cmake.org/download/)
* [Ninja](https://ninja-build.org/)
* [vcpkg](https://github.com/microsoft/vcpkg)
* C++ compiler with C++ 20 support

Once all requirements are installed:

```
git clone --recurse-submodules https://github.com/sarpsenturk/orion-engine
cd orion-engine
mkdir build && cd build
cmake --preset=<selected-preset> ..
```

## Platform Support
The goal is for Orion to be fully cross-platform.
Here's the list of platforms that are planned to be supported:
- [x] Windows 10/11
- [ ] Linux
- [ ] macOS

Only x86 is supported at this time however ARM support is
planned for flavors of Linux, Apple M1, M2 chips and mobile.


## Graphics API Support
The plan to stick with Vulkan for the foreseeable future since it 
allows for rendering on multiple platforms.

However native APIs will be supported in the future. That means
DirectX11/12 for Windows, Metal for Apple devices.
