cmake_minimum_required(VERSION 3.26)
project(csharpify)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

find_program(DOTNET "dotnet")
if ("${DOTNET}" STREQUAL "DOTNET-NOTFOUND")
  message(FATAL_ERROR "dotnet could not be found!")
else()
  execute_process(COMMAND "${DOTNET}" --version
                  OUTPUT_VARIABLE DOTNET_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

set(ORIGINAL_PATH $ENV{PATH})

set(DOTNET_VERSION ${DOTNET_VERSION})

# Find dotnet versions
cmake_path(GET DOTNET PARENT_PATH DOTNET_PATH)
cmake_path(APPEND DOTNET_PATH "sdk/${DOTNET_VERSION}" OUTPUT_VARIABLE DOTNET_PATH)
cmake_path(APPEND DOTNET_PATH "Microsoft.NETCoreSdk.BundledVersions.props" OUTPUT_VARIABLE VERSIONS_PROPS)
file(READ "${VERSIONS_PROPS}" VersionProps_CONTENT)
string(REGEX MATCH "<BundledNETCoreAppPackageVersion>(.*)</BundledNETCoreAppPackageVersion>" DOTNET_SDK_VERSION "${VersionProps_CONTENT}")
set(DOTNET_SDK_VERSION "${CMAKE_MATCH_1}")
message(STATUS "Using ${DOTNET} version ${DOTNET_VERSION} sdk ${DOTNET_SDK_VERSION}")

string(REGEX MATCH "([0-9]+)\.([0-9]+)\\.([0-9]+)" VERSION_PARTS "${DOTNET_VERSION}")
set(DOTNET_MAJOR "${CMAKE_MATCH_1}")
set(DOTNET_MINOR "${CMAKE_MATCH_2}")
set(DOTNET_PATCH "${CMAKE_MATCH_3}")
message(STATUS "${DOTNET_SDK_VERSION} ${DOTNET_VERSION} ${DOTNET_MAJOR} ${DOTNET_MINOR} ${DOTNET_PATCH}")


if(NOT "${DOTNET_ARCH}")
  set(DOTNET_ARCH "${CMAKE_SYSTEM_PROCESSOR}" CACHE STRING ".NET Arch" FORCE)
else(if NOT DEFINED CACHE{DOTNET_ARCH})
  set(DOTNET_ARCH "${DOTNET_ARCH}" CACHE STRING ".NET Arch")
endif()

if(NOT DOTNET_PLATFORM)
  if(CMAKE_HOST_APPLE)
    set(DOTNET_PLATFORM "osx" CACHE STRING ".NET Platform" FORCE)
  elseif(CMAKE_HOST_WIN32)
    set(DOTNET_PLATFORM "win" CACHE STRING ".NET Platform" FORCE)
  else()
    set(DOTNET_PLATFORM "linux" CACHE STRING ".NET Platform" FORCE)
  endif()
else(if NOT DEFINED CACHE{DOTNET_PLATFORM})
  set(DOTNET_PLATFORM "${DOTNET_PLATFORM}" CACHE STRING ".NET Platform")
endif()

if(DOTNET_PLATFORM STREQUAL "osx" OR DOTNET_PLATFORM STREQUAL "win" OR DOTNET_PLATFORM STREQUAL "linux")
  set(DEFAULT_RUNTIME "coreclr")
else()
  set(DEFAULT_RUNTIME "mono")
endif()

if(NOT DOTNET_RUNTIME)
  set(DOTNET_RUNTIME "${DEFAULT_RUNTIME}" CACHE STRING ".NET Runtime" FORCE)
else(if NOT DEFINED CACHE{DOTNET_RUNTIME})
  set(DOTNET_RUNTIME "${DOTNET_RUNTIME}" CACHE STRING ".NET Runtime")
endif()

if(DOTNET_PLATFORM STREQUAL "osx")
  find_library(IOKIT IOKit)
  find_library(COCOA Cocoa)
  find_library(COREVIDEO CoreVideo)
elseif(DOTNET_PLATFORM STREQUAL "win")
elseif(DOTNET_PLATFORM STREQUAL "linux")
endif()

set(RID "${DOTNET_PLATFORM}-${DOTNET_ARCH}")

message(STATUS ".NET RID: ${RID}")
message(STATUS ".NET Runtime: ${DOTNET_RUNTIME}")

if (DOTNET_RUNTIME STREQUAL "coreclr")
  set(runtimesuffix "")
elseif(DOTNET_RUNTIME STREQUAL "mono")
  set(runtimesuffix "Mono.")
else()
    set(runtimesuffix "NativeAOT.")
endif()

set(DOTNET_SDK_PATH_ROOT
  "${CMAKE_SOURCE_DIR}/.packages/microsoft.netcore.app.runtime.${runtimesuffix}${RID}/${DOTNET_SDK_VERSION}/runtimes/${RID}"
  CACHE STRING ".NET SDK root"
)

set(DOTNET_SDK_PATH "${DOTNET_SDK_PATH_ROOT}/lib/net${DOTNET_MAJOR}.${DOTNET_MINOR}" CACHE STRING ".NET SDK path")
set(DOTNET_LIBRARY_PATH "${DOTNET_SDK_PATH_ROOT}/native" CACHE STRING ".NET lib path for linking")

find_package(SDL2 REQUIRED)
message(STATUS "${SDL2_INCLUDE_DIRS} ${SDL2_LIBRARIES}")

find_package(Vulkan REQUIRED COMPONENTS MoltenVK)
message(STATUS "${Vulkan_MoltenVK_LIBRARY} ${Vulkan_LIBRARY} ${Vulkan_INCLUDE_DIR}")

if(DOTNET_RUNTIME STREQUAL "coreclr")
  find_library(CORECLR coreclr PATHS "${DOTNET_LIBRARY_PATH}")
  message(STATUS "${CORECLR} ${DOTNET_LIBRARY_PATH}")
endif()

add_subdirectory(dotnet-deps EXCLUDE_FROM_ALL)
add_subdirectory(managed)
add_subdirectory(native)

add_custom_target(program ALL)
add_dependencies(program
  packages
  csharpify_managed_bundled
  csharpify_native_bundled
)
