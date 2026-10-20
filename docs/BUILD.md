# Building

## Configure and build

From the repository root, after shell initialization:

```powershell
cmake --list-presets
cmake --preset debug-x64-msvc-windows
cmake --build build/debug-x64-msvc-windows
```

All presets follow the naming convention of `[type]-[arch]-[compiler]-[os]`.
All presets output to `build/<preset>`.
Currently only x64 MSVC Windows builds are supported.

## Run and verify

```powershell
ctest --test-dir build/debug-x64-msvc-windows --output-on-failure
./build/debug-x64-msvc-windows/examples/orion.sandbox.exe
```

For most changes a compiling project and passing tests are enough.

If the changes require visual confirmation or interactive testing, the sandbox app (or whichever executable is being worked on) can be used.

## Dependencies

Dependencies should be managed through vcpkg via [vcpkg.json](.vcpkg.json) unless explicitly stated otherwise here:

- ImGui: vendored inside `engine/imgui`
- GoogleTest: fetched at configure time inside [cmake/gtest.cmake](../cmake/gtest.cmake)

## Visual Studio developer shell

On the current Windows machine, CMake, clang-format, and other development tools belong to the Visual Studio environment and are not available from the default shell. Initialize an x64 developer environment before invoking build or formatting tools. Missing commands in an ordinary shell do not establish that the tools are uninstalled.

Either open Developer PowerShell for Visual Studio or initialize an existing PowerShell session from the repository root:

```powershell
$orionVsWhere = "${env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe"
$orionVsPath = & $orionVsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
& "$orionVsPath/Common7/Tools/Launch-VsDevShell.ps1" -Arch amd64 -HostArch amd64 -SkipAutomaticLocation -NoLogo
```

`-SkipAutomaticLocation` preserves the working directory. Shell environment changes apply only to that process and its children: agents using separate shell invocations may need to initialize the environment again in the same invocation as each build command.

In the current setup, clang-format is installed with Visual Studio but remains absent from `PATH` even after developer-shell initialization. Invoke the bundled x64 executable directly, for example:

```powershell
& "$orionVsPath/VC/Tools/Llvm/x64/bin/clang-format.exe" --version
# To format a changed C++ file:
# & "$orionVsPath/VC/Tools/Llvm/x64/bin/clang-format.exe" -i engine/orion/path/to/file.cpp
```
