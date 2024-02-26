project(csharpify_native)
set(CMAKE_CXX_STANDARD 17)

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

set(SOURCES
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

add_executable(csharpify
	${SOURCES}
)

include_directories(csharpify ${PROJECT_SOURCE_DIR} ${generated_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS} "${IMGUI_DIR}" "${IMGUI_DIR}/backends" "${PROJECT_BINARY_DIR}")
target_link_libraries(csharpify ${SDL2_LIBRARIES} ${Vulkan_MoltenVK_LIBRARY} ${Vulkan_LIBRARY} ${IOKIT} ${CORECLR})
add_compile_definitions(csharpify DNNE_COMPILE_AS_SOURCE CORECLR_RUNTIME)
link_directories(csharpify ${DOTNET_LIBRARY_PATH})

add_dependencies(csharpify csharpify_managed)

configure_file(config.h.in config.h)

# Bundle everything into a runnable directory
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_custom_target(csharpify_native_bundled
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:csharpify>" "${CMAKE_BINARY_DIR}/bin/"
  DEPENDS csharpify
)
