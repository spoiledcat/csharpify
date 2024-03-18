cmake_minimum_required(VERSION 3.26)
project(csharpify)
set(version 1.0.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(CSharpifyDefinitions)
include(CSharpifyDotnet)

add_subdirectory(dotnet-deps)

if(RUNTIME_CORECLR AND (NOT DOTNET_PLATFORM STREQUAL "win"))
  find_library(CORECLR coreclr PATHS "${DOTNET_LIBRARY_PATH}")
elseif(RUNTIME_MONO)
  find_library(CORECLR coreclr PATHS "${DOTNET_LIBRARY_PATH}")
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

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

if(BUILD_SDL2)
  set(SDL2_DISABLE_SDL2MAIN 1)
  set(SDL_TEST 0)
  set(BUILD_SHARED_LIBS 0)
  set(SDL_VULKAN 1)
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
endif()

if(DOTNET_PLATFORM STREQUAL "osx" OR DOTNET_PLATFORM STREQUAL "ios")
  set(USE_MOLTENVK 1)
endif()

if(USE_MOLTENVK)
  find_package(Vulkan REQUIRED COMPONENTS MoltenVK)
else()
  find_package(Vulkan REQUIRED)
  set(Vulkan_MoltenVK_LIBRARY "")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "CMAKE_INSTALL_PREFIX is not set. Default value is '${CMAKE_INSTALL_PREFIX}', will set it to ${CMAKE_SOURCE_DIR}/install")
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Where the library will be installed to" FORCE)
endif()

file(MAKE_DIRECTORY packages)
execute_process(
  COMMAND ${CMAKE_COMMAND} 
    -Din_file:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}/cimgui/CMakeLists.txt
    -Dpatch_file:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}/cimgui.patch
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PatchFile.cmake
)

set(IMGUI_STATIC 0 CACHE BOOL "" FORCE)
set(IMGUI_OBJECT 1 CACHE BOOL "" FORCE)

add_subdirectory(managed)
add_subdirectory(cimgui)
add_subdirectory(native)

install(TARGETS csharpify_coreclr EXPORT csharpify_coreclr_export
  FILE_SET HEADERS DESTINATION include/csharpify/coreclr
)

install(TARGETS csharpify_mono EXPORT csharpify_mono_export
  FILE_SET HEADERS DESTINATION include/csharpify/mono
)

install(DIRECTORY ${csharpify_native_SOURCE_DIR}/coreclr/ DESTINATION include/csharpify/coreclr
        FILES_MATCHING PATTERN "*.cpp")

install(DIRECTORY ${csharpify_native_SOURCE_DIR}/mono/ DESTINATION include/csharpify/mono
        FILES_MATCHING PATTERN "*.cpp")

install(EXPORT csharpify_coreclr_export
  FILE CSharpifyCoreCLR.cmake
  DESTINATION lib/cmake/csharpify
)

install(EXPORT csharpify_mono_export
  FILE CSharpifyMono.cmake
  DESTINATION lib/cmake/csharpify
)

install(FILES cmake/CSharpifyDefinitions.cmake DESTINATION lib/cmake/csharpify)
install(FILES cmake/CSharpifyDotnet.cmake DESTINATION lib/cmake/csharpify)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/dotnet-deps/ DESTINATION lib/cmake/csharpify/dotnet-deps
        FILES_MATCHING PATTERN "*.csproj" PATTERN "CMakeLists.txt"
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/dotnet-deps/imports/ DESTINATION lib/cmake/csharpify/dotnet-deps/imports/
        FILES_MATCHING PATTERN "*.props"
)

install(FILES native/config.h.in DESTINATION lib/cmake/csharpify)

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/CSharpifyConfig.cmake"
  INSTALL_DESTINATION lib/cmake/csharpify
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/CSharpifyConfigVersion.cmake"
  VERSION "${version}"
  COMPATIBILITY AnyNewerVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/CSharpifyConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/CSharpifyConfigVersion.cmake"
  DESTINATION lib/cmake/csharpify
)

