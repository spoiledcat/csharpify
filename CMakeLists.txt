cmake_minimum_required(VERSION 3.26)
project(csharpify)

add_custom_target(program ALL)

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

add_subdirectory(dotnet-deps EXCLUDE_FROM_ALL)

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
message(STATUS ".NET SDK:${DOTNET_SDK_VERSION}")
message(STATUS ".NET Version:${DOTNET_MAJOR}.${DOTNET_MINOR}.${DOTNET_PATCH}")

message(STATUS "DOTNET_ARCH:${DOTNET_ARCH}")

if(NOT DOTNET_ARCH)
  string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" DOTNET_ARCH)
  string(REPLACE "amd" "x" DOTNET_ARCH "${DOTNET_ARCH}")
  set(DOTNET_ARCH "${DOTNET_ARCH}" CACHE STRING ".NET Arch" FORCE)
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
  set(BUILD_SDL2 1)
  # there are two libs, nethost.lib (dynamic) and libnethost.lib (static), and we want the static one
  set(NETHOST "${DOTNET_APPHOST_PATH}/libnethost.lib")
elseif(DOTNET_PLATFORM STREQUAL "linux")
endif()

set(RID "${DOTNET_PLATFORM}-${DOTNET_ARCH}" CACHE STRING ".NET Runtime Identifier" FORCE)

message(STATUS ".NET RID: ${RID}")
message(STATUS ".NET Runtime: ${DOTNET_RUNTIME}")

set(RUNTIME_CORECLR 0)
set(RUNTIME_MONO 0)
set(RUNTIME_NATIVEAOT 0)

if (DOTNET_RUNTIME STREQUAL "coreclr")
  set(RUNTIME_CORECLR 1)
  set(runtimesuffix "")
elseif(DOTNET_RUNTIME STREQUAL "mono")
  set(RUNTIME_MONO 1)
  set(runtimesuffix "Mono.")
else()
  set(RUNTIME_NATIVEAOT 1)
  set(runtimesuffix "NativeAOT.")
endif()

cmake_path(APPEND CMAKE_SOURCE_DIR ".packages" OUTPUT_VARIABLE DOTNET_PACKAGES_PATH)

# this is useful on all platforms for the coreclr definitions
set(DOTNET_APPHOST_PATH
  "${DOTNET_PACKAGES_PATH}/microsoft.netcore.app.host.win-x64/${DOTNET_SDK_VERSION}/runtimes/win-x64/native"
  CACHE STRING ".NET App Host" FORCE
)

set(DOTNET_SDK_PATH_ROOT
  "${DOTNET_PACKAGES_PATH}/microsoft.netcore.app.runtime.${runtimesuffix}${RID}/${DOTNET_SDK_VERSION}/runtimes/${RID}"
  CACHE STRING ".NET SDK root" FORCE
)

set(DOTNET_SDK_PATH "${DOTNET_SDK_PATH_ROOT}/lib/net${DOTNET_MAJOR}.${DOTNET_MINOR}" CACHE STRING ".NET SDK path" FORCE)
set(DOTNET_LIBRARY_PATH "${DOTNET_SDK_PATH_ROOT}/native" CACHE STRING ".NET lib path for linking" FORCE)

if(NOT DOTNET_INCLUDE_DIRS)
  set(DOTNET_INCLUDE_DIRS "")
endif()

if(RUNTIME_MONO)
  list(APPEND DOTNET_INCLUDE_DIRS "${DOTNET_LIBRARY_PATH}/include/mono-2.0/")
endif()

unset(CORECLR CACHE)

if(RUNTIME_CORECLR AND (NOT DOTNET_PLATFORM STREQUAL "win"))
  find_library(CORECLR coreclr PATHS "${DOTNET_LIBRARY_PATH}")
  message(STATUS "${CORECLR} ${DOTNET_LIBRARY_PATH}")
elseif(RUNTIME_MONO)
  find_library(CORECLR coreclr PATHS "${DOTNET_LIBRARY_PATH}")
  message(STATUS "${CORECLR} ${DOTNET_LIBRARY_PATH}")
endif()

if(BUILD_SDL2)
  set(SDL2_DISABLE_SDL2MAIN 1)
  set(SDL_TEST 0)
  set(BUILD_SHARED_LIBS 0)
  set(SDL_VULKAN 0)
  set(SDL_DIRECTX 0)
  include(FetchContent)
  FetchContent_Declare(
    sdl2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    GIT_TAG        SDL2
  )
  FetchContent_MakeAvailable(sdl2)

  #set(SDL2_INCLUDE_DIRS "${SDL2_BINARY_DIR}/include" "${SDL2_BINARY_DIR}/include/SDL2")

elseif(DOTNET_PLATFORM STREQUAL "win")
  include(FetchContent)
  FetchContent_Declare(
    sdl2
    URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.0/SDL2-devel-2.30.0-VC.zip
    URL_HASH MD5=179d5f19c62c16343a3bd606ea5b121a
  )
  FetchContent_MakeAvailable(sdl2)

  list(APPEND CMAKE_PREFIX_PATH "${sdl2_SOURCE_DIR}")

  set(HAVE_SDL2_PACKAGE 1)
else()
  set(HAVE_SDL2_PACKAGE 1)
endif()

if(HAVE_SDL2_PACKAGE)
  find_package(SDL2 REQUIRED)
  message(STATUS "${SDL2_INCLUDE_DIRS} ${SDL2_LIBRARIES}")
endif()

if(DOTNET_PLATFORM STREQUAL "osx" OR DOTNET_PLATFORM STREQUAL "ios")
  set(USE_MOLTENVK 1)
endif()

if(USE_MOLTENVK)
  find_package(Vulkan REQUIRED COMPONENTS MoltenVK)
  message(STATUS "${Vulkan_MoltenVK_LIBRARY} ${Vulkan_LIBRARY} ${Vulkan_INCLUDE_DIR}")
else()
  find_package(Vulkan REQUIRED)
  message(STATUS "${Vulkan_LIBRARY} ${Vulkan_INCLUDE_DIR}")
  set(Vulkan_MoltenVK_LIBRARY "")
endif()

set(IMGUI_STATIC 1)

make_directory(packages)
execute_process(
  COMMAND ${CMAKE_COMMAND} 
    -Din_file:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}/cimgui/CMakeLists.txt
    -Dpatch_file:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}/cimgui.patch
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PatchFile.cmake
)

add_subdirectory(managed)
add_subdirectory(cimgui)
add_subdirectory(native)

add_dependencies(program
  packages
  csharpify_managed_bundled
  csharpify_native_bundled
)
