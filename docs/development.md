# Development

## Prerequisites

The current development target is Windows x64 with MSVC and C++23 support. Install Visual Studio's C++ desktop tooling, CMake 3.30 or newer, and Ninja. Use the checked-in presets in [CMakePresets.json](../CMakePresets.json).

Dependencies come from three places:

- vcpkg provides GLFW, GLM, and spdlog, with versions and the registry baseline recorded in [vcpkg.json](../vcpkg.json) and [vcpkg-configuration.json](../vcpkg-configuration.json).
- The Vulkan SDK provides Vulkan headers/libraries, Volk, and the SDK headers used by the backend, including Vulkan Memory Allocator. `slangc` must be discoverable on `PATH`; the current setup uses the SDK's copy.
- GoogleTest is fetched at configure time at the revision pinned in [cmake/gtest.cmake](../cmake/gtest.cmake). Dear ImGui is vendored in `engine/imgui/`.

Initial configuration may need network access for vcpkg and GoogleTest. Running the sandbox also requires a GPU/driver supporting the features enabled by the Vulkan backend; see [renderer](renderer.md).

## Visual Studio developer shell

On the current Windows machine, CMake, clang-format, and other development tools belong to the Visual Studio environment and are not available from the default shell. Initialize an x64 developer environment before invoking build or formatting tools. Missing commands in an ordinary shell do not establish that the tools are uninstalled.

Either open Developer PowerShell for Visual Studio or initialize an existing PowerShell session from the repository root:

```powershell
$orionVsWhere = "${env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe"
$orionVsPath = & $orionVsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
& "$orionVsPath/Common7/Tools/Launch-VsDevShell.ps1" -Arch amd64 -HostArch amd64 -SkipAutomaticLocation -NoLogo
```

`-SkipAutomaticLocation` preserves the working directory. Shell environment changes apply only to that process and its children: agents using separate shell invocations may need to initialize the environment again in the same invocation as each build command.

The presets require `VCPKG_ROOT` to point to the vcpkg installation. If it is unset and using Visual Studio's bundled vcpkg, set it for the session after resolving `$orionVsPath` above:

```powershell
$env:VCPKG_ROOT = "$orionVsPath/VC/vcpkg"
Get-Command cmake, ctest, ninja, cl, slangc
```

For a separate vcpkg installation, use its root instead. Ensure the Vulkan SDK environment is available too.

In the current setup, clang-format is installed with Visual Studio but remains absent from `PATH` even after developer-shell initialization. Invoke the bundled x64 executable directly, for example:

```powershell
& "$orionVsPath/VC/Tools/Llvm/x64/bin/clang-format.exe" --version
# To format a changed C++ file:
# & "$orionVsPath/VC/Tools/Llvm/x64/bin/clang-format.exe" -i engine/orion/path/to/file.cpp
```

If a particular tool remains unresolved, locate it within the installed Visual Studio components; avoid assuming a specific Visual Studio version or global `PATH` configuration.

## Configure and build

From the repository root, after shell initialization:

```powershell
cmake --list-presets
cmake --preset debug-x64-msvc-windows
cmake --build build/debug-x64-msvc-windows
```

The other checked-in preset is `release-x64-msvc-windows`, which selects `RelWithDebInfo`. Substitute that preset name in both the configure command and build directory to use it. These are configure presets; there are currently no build or test presets.

Both presets use Ninja, with output under `build/<preset-name>/`. Useful configuration options:

| Option | Default when building this repository directly | Purpose |
| --- | --- | --- |
| `ORION_BUILD_EXAMPLES` | `ON` | Build `orion.sandbox`. |
| `ORION_BUILD_TESTS` | `ON` | Fetch GoogleTest and build/discover engine tests. |
| `ORION_SHADER_OUTPUT_DIR` | `<build-dir>/engine/shaders` | Compiled SPIR-V output. |

Pass overrides at configure time, for example `cmake --preset debug-x64-msvc-windows -DORION_BUILD_EXAMPLES=OFF`. Tests and examples default to off when Orion is included as a subproject. Shader compilation is a dependency of `orion.engine`.

## Run and verify

```powershell
ctest --test-dir build/debug-x64-msvc-windows --output-on-failure
./build/debug-x64-msvc-windows/examples/orion.sandbox.exe
```

The sandbox currently displays the ImGui demo over a cleared background. For windowing or rendering changes, check drawing, input, resizing, minimizing/restoring, detached ImGui windows when relevant, and closing the application. Running from the executable's output directory keeps generated files such as `imgui.ini` under the ignored build tree.

Tests currently cover hashing and `InlineFunction`; passing them does not verify rendering. Tests live in `engine/tests/`, are registered in its `CMakeLists.txt` through `orion_add_test`, and are discovered with GoogleTest by CTest. Build before testing so results reflect the current source. Run relevant tests and add meaningful regression coverage when appropriate to the change.

Use `.clang-format` for C++ formatting and `.clang-tidy` for the configured analysis checks. MSVC engine compilation enables `/W4 /WX /permissive-`; preserve warnings-as-errors. Source files and public headers are explicitly listed in `engine/CMakeLists.txt`, so register new files there.
