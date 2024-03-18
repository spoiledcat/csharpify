# CSharpify

An example of how to embed C# in a C/C++ application, supporting Mono and CoreCLR on multiple platforms.

This example initializes and runs [DearImGui](https://github.com/ocornut/imgui) with the SDL2+Vulkan backend, by taking the corresponding example source and replacing everything between `ImGui::NewFrame()` and `ImGui::Render()` with a call back to the csharpify main code, which then calls a C# method, on every frame. cimgui and the DearImGui C# bindings are included, so C# can call all the DearImGui APIs that the demo would normally call.

The C declarations and trampoline methods are generated using a slightly modified version of [DNNE](https://github.com/shana/DNNE), a prototype project that generates native exports for C#.

## How to build

Requirements:
- Cmake 3.20+
- .NET 8.0.101 - This is currently enforced with global.json, but there's no reason it shouldn't work with any .NET 7+

After cloning this repository, grab the submodules.

```
git submodule update --init
```

After building, all needed files should be in `build/[runtime]/native/bin`, where `runtime` is the runtime corresponding to the preset you chose. On Windows, it will be in `build/[runtime]/native/bin/Debug`, due to the way Visual Studio creates output directories.

### Mono

```
cmake --preset mono
cmake --build --preset mono
```

### CoreCLR

```
cmake --preset coreclr
cmake --build --preset coreclr
```

### AOT/NativeAOT

TODO: Work in progress, coming soon.