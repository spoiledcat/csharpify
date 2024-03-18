# CSharpify

An example of how to embed C# in a C/C++ application, supporting Mono and CoreCLR on multiple platforms.

This example initializes and runs [DearImGui](https://github.com/ocornut/imgui) with the SDL2+Vulkan backend, by taking the corresponding example source and replacing everything between `ImGui::NewFrame()` and `ImGui::Render()` with a call back to the csharpify main code, which then calls a C# method, on every frame. cimgui and the DearImGui C# bindings are included, so C# can call all the DearImGui APIs that the demo would normally call.

The C declarations and trampoline methods are generated using a slightly modified version of [DNNE](https://github.com/shana/DNNE), a prototype project that generates native exports for C#.

## How to build

Requirements:
- Cmake 3.26+
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


## How to use this in your own project

Running

```
cmake --install build/[runtime]
```

will create an install directory with the isolated header and source files and cmake files needed, without the DearImGui bits. You can use either runtime preset for this, files for all the runtime configurations will be included.

### Example of how to include csharpify in another CMake project

A demo project with the files configured as below is in https://github.com/spoiledcat/csharpify-demo

Add a NuGet.config file to the root of your project, so that the .NET runtime libraries can be in a known location for linking.

```NuGet.config
<?xml version="1.0" encoding="utf-8"?>
<configuration>
  <config>
    <!-- use a local package folder because we produce nuget packages which may end up having to be cleaned,
         and it's much easier to clean a local repository than the global nuget cache (in particular on bots) -->
    <add key="repositorypath" value=".packages" />
    <add key="globalPackagesFolder" value=".packages" />
  </config>
  <packageSources>
    <add key="local" value="packages" />
  </packageSources>
  <disabledPackageSources>
  </disabledPackageSources>
</configuration>
```

Create a `packages` folder and copy over the `DNNE.2.0.6.2.nupkg` package into it, from the packages folder in this repository.

Configure your CMakeLists.txt file to find the csharpify package, set up the dotnet-deps project and config.h, include paths and link flags.

```CMakeLists.txt
cmake_minimum_required(VERSION 3.26)
project(test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

find_package(CSharpify HINTS "${CMAKE_SOURCE_DIR}/../csharpify/install/lib/cmake")

message(STATUS "${CMAKE_MODULE_PATH}")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/packages")
add_subdirectory("${CMAKE_SOURCE_DIR}/dotnet-deps")

# This must come after including dotnet-deps
include(CSharpifyDotnet)

configure_file("${CSHARPIFY_CONFIG_H_IN}" config.h)

list(APPEND dependencies packages)

# Configuring paths and file lists for the dotnet build and publish steps
cmake_path(APPEND PROJECT_SOURCE_DIR "main.csproj" OUTPUT_VARIABLE CSPROJ)
cmake_path(GET CSPROJ STEM ASSEMBLYNAME)
set(ASSEMBLYNAME "${ASSEMBLYNAME}" CACHE STRING "Assembly name")

list(APPEND build_outputs ${ASSEMBLYNAME}.dll ${ASSEMBLYNAME}.pdb)
if(DOTNET_PLATFORM STREQUAL "win" AND DOTNET_RUNTIME STREQUAL "coreclr")
  list(APPEND build_outputs ${ASSEMBLYNAME}.runtimeconfig.json)
endif()

set(publish_args "")
set(publish_dir "${PROJECT_BINARY_DIR}/publish")
set(bin_dir "${CMAKE_BINARY_DIR}")
if(DOTNET_PLATFORM STREQUAL "win")
  set(bin_dir "${CMAKE_BINARY_DIR}/Debug")
endif()

file(GLOB cs_sources "${PROJECT_SOURCE_DIR}/*.cs")

# Run dotnet build
add_custom_command(
  OUTPUT "${PROJECT_BINARY_DIR}/.stamp" ${build_outputs} "${PROJECT_SOURCE_DIR}/generated/exports.g.c"
  COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} build \"${CSPROJ}\" --nologo -c $<CONFIG> -o \"${PROJECT_BINARY_DIR}\" /p:UseMonoRuntime=$<IF:$<BOOL:${RUNTIME_MONO}>,true,false>
  COMMAND ${CMAKE_COMMAND} -E touch "${PROJECT_BINARY_DIR}/.stamp"
  BYPRODUCTS "${PROJECT_SOURCE_DIR}/generated/dnne.h" "${PROJECT_SOURCE_DIR}/generated/exports.h" "${PROJECT_SOURCE_DIR}/generated/platform.c"
  DEPENDS "Directory.Build.props" "Directory.Build.targets" ${cs_sources} "${CSPROJ}"
)
add_custom_target(build DEPENDS ${build_outputs} "${PROJECT_SOURCE_DIR}/generated/exports.g.c")

# Run dotnet publish
add_custom_command(
  OUTPUT "${publish_dir}/.stamp" "${publish_dir}/${ASSEMBLYNAME}.dll" "${publish_dir}/System.dll"
  COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} publish \"${CSPROJ}\" ${publish_args} --sc -r ${RID} --nologo -c $<CONFIG> -o \"${publish_dir}\"  /p:UseMonoRuntime=$<IF:$<BOOL:${RUNTIME_MONO}>,true,false>
  COMMAND ${CMAKE_COMMAND} -E touch "${publish_dir}/.stamp"
  DEPENDS "Directory.Build.props" "Directory.Build.targets" ${cs_sources} "${CSPROJ}"
)
add_custom_target(publish DEPENDS "${publish_dir}/${ASSEMBLYNAME}.dll")

if(NOT(DOTNET_PLATFORM STREQUAL "win" AND DOTNET_RUNTIME STREQUAL "coreclr"))
  add_custom_target(link_sdk
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${publish_dir}" "${bin_dir}/sdk"
    DEPENDS "${publish_dir}/System.dll"
  )
  list(APPEND dependencies link_sdk)
endif()


list(APPEND export_sources "${PROJECT_SOURCE_DIR}/generated/exports.g.c")
list(APPEND export_includes "${PROJECT_SOURCE_DIR}/generated")

if(DOTNET_PLATFORM STREQUAL "win")
  list(APPEND export_sources "${PROJECT_SOURCE_DIR}/generated/platform.c")
endif()

add_executable(test main.cpp ${export_sources})
target_include_directories(test PRIVATE "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_BINARY_DIR}/include" "${DOTNET_INCLUDE_DIRS}" ${export_includes} )
target_compile_definitions(test PRIVATE DNNE_COMPILE_AS_SOURCE DNNE_SELF_CONTAINED_RUNTIME)

if(RUNTIME_MONO)
  list(APPEND dependencies csharpify_mono)
  target_link_libraries(test PRIVATE csharpify_mono ${CORECLR})
elseif(RUNTIME_CORECLR)
  list(APPEND dependencies csharpify_coreclr)
  target_link_libraries(test PRIVATE csharpify_coreclr ${CORECLR})
endif()

add_dependencies(test packages ${dependencies})
```

Include the headers in your source.

```main.cpp
#include "config.h"
#include "common.h"
#include "managed_bridge.h"
#include "managed_exports.h"

int main(int argc, char** argv)
{
}
```