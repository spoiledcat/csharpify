project(csharpify_native)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

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

list(APPEND SOURCES
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


if(DOTNET_RUNTIME STREQUAL "coreclr")
	if(DOTNET_PLATFORM STREQUAL "win")
		list(APPEND SOURCES ${CMAKE_SOURCE_DIR}/.packages/dnne/2.0.6/tools/platform/platform.c)
	endif()
	list(APPEND SOURCES coreclr/coreclr.cpp)
endif()

configure_file(config.h.in include/config.h)

add_executable(csharpify ${SOURCES})

target_sources(csharpify
	PRIVATE FILE_SET HEADERS
	BASE_DIRS include ${PROJECT_BINARY_DIR}/include
	FILES
	include/imgui_bridge.h
	include/main.h
	include/managed_exports.h
	include/managed_bridge.h
	include/utils.h
	${PROJECT_BINARY_DIR}/include/config.h
)


if(DOTNET_RUNTIME STREQUAL "coreclr")
	target_sources(csharpify
		PRIVATE FILE_SET HEADERS
		BASE_DIRS coreclr
		FILES
		coreclr/bridge.h
		coreclr/coreclrhost.h
		coreclr/runtime.h
	)
endif()

include_directories(csharpify 
	${generated_INCLUDE_DIR} 
	${SDL2_INCLUDE_DIRS} 
	${Vulkan_INCLUDE_DIRS} 
	"${IMGUI_DIR}" 
	"${IMGUI_DIR}/backends"
)

target_link_libraries(csharpify 
	${SDL2_LIBRARIES} 
	${Vulkan_LIBRARY}
)

if(DOTNET_PLATFORM STREQUAL "win")
	target_compile_definitions(csharpify PRIVATE "DNNE_ASSEMBLY_NAME=${ASSEMBLYNAME}")
	target_compile_definitions(csharpify PRIVATE NETHOST_USE_AS_STATIC)
	set_property(TARGET csharpify PROPERTY
             MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	include_directories(csharpify 
		${DOTNET_APPHOST_PATH}
	)
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

add_compile_definitions(csharpify DNNE_COMPILE_AS_SOURCE CORECLR_RUNTIME)
link_directories(csharpify ${DOTNET_LIBRARY_PATH})

add_dependencies(csharpify csharpify_managed)
set(CSHARPIFY_FILE "${CMAKE_EXECUTABLE_PREFIX}csharpify${CMAKE_EXECUTABLE_SUFFIX}")
get_target_property(SDL2_DLL SDL2::SDL2 "IMPORTED_LOCATION")

list(APPEND build_outputs
	"${SDL2_DLL}"
)

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

add_custom_target(csharpify_native_bundled DEPENDS "${bundle_outputs}")

add_custom_target(run
	COMMAND "${CMAKE_BINARY_DIR}/bin/${CSHARPIFY_FILE}"
	DEPENDS "${CMAKE_BINARY_DIR}/bin/${CSHARPIFY_FILE}"
)
