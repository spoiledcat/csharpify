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

list(APPEND coreclr_shared_sources
	coreclr/coreclr.cpp
	coreclr/coreclr_win.cpp)

list(APPEND mono_shared_sources
	mono/mono.cpp
)

list(APPEND csharpify_sources
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

if(RUNTIME_CORECLR AND DOTNET_PLATFORM STREQUAL "win")
	list(APPEND csharpify_sources ${generated_INCLUDE_DIR}/platform.c)
endif()

configure_file(config.h.in include/config.h)

add_library(csharpify_coreclr INTERFACE)
add_library(csharpify_mono INTERFACE)

add_executable(csharpify ${csharpify_sources})

if(RUNTIME_CORECLR)
	target_link_libraries(csharpify csharpify_coreclr)
elseif(RUNTIME_MONO)
	target_link_libraries(csharpify csharpify_mono)
elseif(RUNTIME_NATIVEAOT)
endif()

add_dependencies(csharpify packages)

add_dependencies(csharpify cimgui)

target_link_libraries(csharpify $<TARGET_OBJECTS:cimgui>)

if(BUILD_SDL2)
	target_link_libraries(csharpify SDL2-static)
	add_dependencies(csharpify SDL2-static)
else()
	target_link_libraries(csharpify ${SDL2_LIBRARIES})
endif()


list(APPEND headers_base_dirs
	include
	${SHARED_INCLUDE_DIRS}
)

list(APPEND headers_shared
	include/common.h
	include/managed_exports.h
	include/managed_bridge.h
	include/utils.h
	include/coreclrhost.h
	${SHARED_HEADERS}
)

target_sources(csharpify
	PRIVATE FILE_SET HEADERS
	BASE_DIRS ${headers_base_dirs} "${PROJECT_BINARY_DIR}/include" "${generated_INCLUDE_DIR}"
	FILES
	${headers_shared}
	include/imgui_bridge.h
	include/main.h
	"${PROJECT_BINARY_DIR}/include/config.h"
	"${generated_INCLUDE_DIR}/dnne.h"
	"${generated_INCLUDE_DIR}/exports.h"
)

target_sources(csharpify_coreclr INTERFACE
	FILE_SET HEADERS
	BASE_DIRS ${headers_base_dirs} coreclr
	FILES
	${headers_shared}
	coreclr/bridge.h
	coreclr/runtime.h
)

target_sources(csharpify_mono INTERFACE
	FILE_SET HEADERS
	BASE_DIRS ${headers_base_dirs} mono
	FILES
	${headers_shared}
	mono/bridge.h
	mono/runtime.h
)

foreach(thing ${coreclr_shared_sources})
	list(APPEND coreclr_export_sources
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${thing}>"
		"$<INSTALL_INTERFACE:include/csharpify/${thing}>"
	)
endforeach()

foreach(thing ${mono_shared_sources})
	list(APPEND mono_export_sources
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${thing}>"
		"$<INSTALL_INTERFACE:include/csharpify/${thing}>"
	)
endforeach()

target_sources(csharpify_coreclr INTERFACE ${coreclr_export_sources})
target_sources(csharpify_mono INTERFACE ${mono_export_sources})

if(RUNTIME_CORECLR)
	target_sources(csharpify
		PRIVATE FILE_SET HEADERS
		BASE_DIRS coreclr
		FILES
		coreclr/bridge.h
		coreclr/runtime.h
	)
else(RUNTIME_MONO)
	target_sources(csharpify
		PRIVATE FILE_SET HEADERS
		BASE_DIRS mono
		FILES
		mono/bridge.h
		mono/runtime.h
	)
endif()

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
	)
endif()

if(CORECLR)
	target_link_libraries(csharpify ${CORECLR})
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

add_custom_command(
	OUTPUT  ${bundle_outputs}
	COMMAND ${CMAKE_COMMAND} -E copy -t "${CMAKE_BINARY_DIR}/bin/" "$<TARGET_FILE:csharpify>" ${build_outputs}
	DEPENDS ${build_outputs}
)

if(DOTNET_PLATFORM STREQUAL "win")
	list(APPEND symbols "$<$<CONFIG:Debug>:${PROJECT_BINARY_DIR}/Debug/${CMAKE_EXECUTABLE_PREFIX}csharpify.pdb>")
	list(APPEND bundle_symbols "$<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/bin/${CMAKE_EXECUTABLE_PREFIX}csharpify.pdb>")

	add_custom_command(
			OUTPUT  ${bundle_symbols}
			COMMAND ${CMAKE_COMMAND} -E copy -t "${CMAKE_BINARY_DIR}/bin/" ${symbols}
			DEPENDS ${symbols}
	)
endif()


add_custom_target(csharpify_native_bundled DEPENDS ${bundle_outputs} ${bundle_symbols})

add_custom_target(run
	COMMAND "${CMAKE_BINARY_DIR}/bin/${CSHARPIFY_FILE}"
	DEPENDS "${CMAKE_BINARY_DIR}/bin/${CSHARPIFY_FILE}"
)

add_custom_target(program DEPENDS csharpify_managed_bundled csharpify_native_bundled)