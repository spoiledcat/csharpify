project(csharpify_native)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

set(CSHARPIFY_FILE "${CMAKE_EXECUTABLE_PREFIX}csharpify${CMAKE_EXECUTABLE_SUFFIX}")

cmake_path(APPEND CMAKE_SOURCE_DIR "imgui" OUTPUT_VARIABLE IMGUI_DIR)

file(READ ${IMGUI_DIR}/examples/example_sdl2_vulkan/main.cpp IMGUI_GEN)
string(LENGTH "${IMGUI_GEN}" IMGUI_GEN_LEN)
string(FIND "${IMGUI_GEN}" "ImGui::NewFrame();" startpos REVERSE)
string(FIND "${IMGUI_GEN}" "ImGui::Render();" endpos REVERSE)
string(SUBSTRING "${IMGUI_GEN}" 0 ${startpos} IMGUI_GEN_HEADER)
string(SUBSTRING "${IMGUI_GEN}" ${endpos} ${IMGUI_GEN_LEN} IMGUI_GEN_FOOTER)
string(JOIN "ImGui::NewFrame();\nMainLoop();\n" IMGUI_GEN "${IMGUI_GEN_HEADER}" "${IMGUI_GEN_FOOTER}")
string(PREPEND IMGUI_GEN "#include \"imgui_bridge.h\"\n")
string(REPLACE "int main(int" "int imgui_main(int" IMGUI_GEN "${IMGUI_GEN}")
file(WRITE ${PROJECT_BINARY_DIR}/imgui.gen.cpp "${IMGUI_GEN}")

set(DOTNET_LIBS "")

if(DOTNET_RUNTIME STREQUAL "coreclr")
	set(DOTNET_LIBS
		coreclr
	)
endif()

list(APPEND SHARED_SOURCES
	main.cpp
	${generated_INCLUDE_DIR}/exports.g.c
	${PROJECT_BINARY_DIR}/imgui.gen.cpp
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
	${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

set(RUNTIME_SOURCES "")

if(RUNTIME_CORECLR)
	if(DOTNET_PLATFORM STREQUAL "win")
		list(APPEND RUNTIME_SOURCES ${generated_INCLUDE_DIR}/platform.c)
	endif()
	list(APPEND RUNTIME_SOURCES coreclr/coreclr.cpp)
elseif(RUNTIME_MONO)
	list(APPEND RUNTIME_SOURCES mono/mono.cpp)
else()
endif()

configure_file(config.h.in include/config.h)

add_executable(csharpify ${SHARED_SOURCES} ${RUNTIME_SOURCES})

# gcc uses -Wl,--whole-archive instead of force_load
# target_link_libraries(csharpify -Wl,-force_load "${cimgui_BINARY_DIR}/cimgui.a")
target_link_libraries(csharpify cimgui SDL2-static)
add_dependencies(csharpify cimgui)
add_dependencies(csharpify SDL2-static)

target_sources(csharpify
	PRIVATE FILE_SET HEADERS
	BASE_DIRS include ${PROJECT_BINARY_DIR}/include ${generated_INCLUDE_DIR}
	FILES
	include/imgui_bridge.h
	include/main.h
	include/managed_exports.h
	include/managed_bridge.h
	include/utils.h
	${PROJECT_BINARY_DIR}/include/config.h
	${generated_INCLUDE_DIR}/dnne.h
	${generated_INCLUDE_DIR}/exports.h
)

target_sources(csharpify
	PRIVATE FILE_SET HEADERS
	BASE_DIRS "${DOTNET_APPHOST_PATH}"
	FILES
	"${DOTNET_APPHOST_PATH}/coreclr_delegates.h"
)

if(RUNTIME_CORECLR)
	target_sources(csharpify
		PRIVATE FILE_SET HEADERS
		BASE_DIRS coreclr
		FILES
		coreclr/bridge.h
		coreclr/coreclrhost.h
		coreclr/runtime.h
	)
else(RUNTIME_MONO)
	target_sources(csharpify
		PRIVATE FILE_SET HEADERS
		BASE_DIRS mono
		FILES
		mono/bridge.h
	)
endif()

message(STATUS "DOTNET_INCLUDE_DIRS: ${DOTNET_INCLUDE_DIRS}")
message(STATUS "generated_INCLUDE_DIR: ${generated_INCLUDE_DIR} ")
message(STATUS "SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")
message(STATUS "Vulkan_INCLUDE_DIRS: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "IMGUI_DIR: ${IMGUI_DIR}")
message(STATUS "IMGUI_DIR/backends: ${IMGUI_DIR}/backends")
message(STATUS "NETHOST: ${NETHOST}")

target_include_directories(csharpify PRIVATE
	${DOTNET_INCLUDE_DIRS}
	${generated_INCLUDE_DIR}
	${SDL2_INCLUDE_DIRS}
	${Vulkan_INCLUDE_DIRS}
	"${IMGUI_DIR}"
	"${IMGUI_DIR}/backends"
	"${CMAKE_SOURCE_DIR}/cimgui"
)

if(BUILD_SDL2)
	target_include_directories(csharpify PRIVATE
		"$<BUILD_INTERFACE:${SDL2_BINARY_DIR}/include>"
		"$<BUILD_INTERFACE:${SDL2_BINARY_DIR}/include/SDL2>"
		"$<BUILD_INTERFACE:${SDL2_BINARY_DIR}/include-config-$<LOWER_CASE:$<CONFIG>>/SDL2>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SDL2>"
	)
else()
	target_include_directories(csharpify PRIVATE
		${SDL2_INCLUDE_DIRS}
	)

endif()

target_link_libraries(csharpify
	#${SDL2_LIBRARIES}
	${Vulkan_LIBRARY}
)

if(DOTNET_PLATFORM STREQUAL "win")
	target_compile_definitions(csharpify PRIVATE "DNNE_ASSEMBLY_NAME=${ASSEMBLYNAME}")
	target_compile_definitions(csharpify PRIVATE NETHOST_USE_AS_STATIC)
	set_property(TARGET csharpify PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	target_link_libraries(csharpify
		${NETHOST}
	)

endif()

if(USE_MOLTENVK)
	target_link_libraries(csharpify
		${Vulkan_MoltenVK_LIBRARY}
	)
endif()

if(DOTNET_PLATFORM STREQUAL "osx")
	target_link_libraries(csharpify
		${IOKIT}
		${CORECLR}
	)
endif()

target_compile_definitions(csharpify PRIVATE DNNE_COMPILE_AS_SOURCE DNNE_SELF_CONTAINED_RUNTIME)

link_directories(csharpify ${DOTNET_LIBRARY_PATH})

add_dependencies(csharpify csharpify_managed)

if(NOT BUILD_SDL2)
	get_target_property(SDL2_DLL SDL2::SDL2 "IMPORTED_LOCATION")

	list(APPEND build_outputs
		"${SDL2_DLL}"
	)
endif()

# Bundle everything into a runnable directory
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

foreach(thing ${build_outputs})
	cmake_path(GET thing FILENAME filename)
	list(APPEND bundle_outputs "${CMAKE_BINARY_DIR}/bin/${filename}")
endforeach()

list(APPEND bundle_outputs "${CMAKE_BINARY_DIR}/bin/${CSHARPIFY_FILE}")

message(STATUS "outputs ${build_outputs}")

add_custom_command(
	OUTPUT  ${bundle_outputs}
	COMMAND ${CMAKE_COMMAND} -E copy -t "${CMAKE_BINARY_DIR}/bin/" "$<TARGET_FILE:csharpify>" ${build_outputs}
	DEPENDS ${build_outputs}
)

list(APPEND symbols "$<$<CONFIG:Debug>:${PROJECT_BINARY_DIR}/Debug/${CMAKE_EXECUTABLE_PREFIX}csharpify.pdb>")
list(APPEND bundle_symbols "$<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/bin/${CMAKE_EXECUTABLE_PREFIX}csharpify.pdb>")
add_custom_command(
	OUTPUT  ${bundle_symbols}
	COMMAND ${CMAKE_COMMAND} -E copy -t "${CMAKE_BINARY_DIR}/bin/" ${symbols}
	DEPENDS ${symbols}
)

add_custom_target(csharpify_native_bundled ALL DEPENDS ${bundle_outputs} ${bundle_symbols})

add_custom_target(run
	COMMAND "${CMAKE_BINARY_DIR}/bin/${CSHARPIFY_FILE}"
	DEPENDS "${CMAKE_BINARY_DIR}/bin/${CSHARPIFY_FILE}"
)
