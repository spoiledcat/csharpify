project(packages)

# ensure packages are restored on first configure
execute_process(
	COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} "restore"
	WORKING_DIRECTORY "${packages_SOURCE_DIR}"
)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/include")
configure_file("${DOTNET_APPHOST_PATH}/coreclr_delegates.h" "${CMAKE_BINARY_DIR}/include/coreclr_delegates.h")

file(GLOB props "${packages_SOURCE_DIR}/imports/*.props")

# # if dependencies change, this will run again at build time
add_custom_command(
	OUTPUT "${packages_SOURCE_DIR}/.stamp"
	"${packages_SOURCE_DIR}/obj/project.assets.json"
	"${CMAKE_BINARY_DIR}/include/coreclr_delegates.h"
	"${DOTNET_SDK_NUSPEC}"
	COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} "restore"
	COMMAND ${CMAKE_COMMAND} -E copy -t "${CMAKE_BINARY_DIR}/include/" "${DOTNET_APPHOST_PATH}/coreclr_delegates.h"
	COMMAND ${CMAKE_COMMAND} -E touch ".stamp"
	WORKING_DIRECTORY "${packages_SOURCE_DIR}"
	DEPENDS packages.csproj ${props}
)

add_custom_target(packages DEPENDS "${CMAKE_BINARY_DIR}/include/coreclr_delegates.h" "${packages_SOURCE_DIR}/obj/project.assets.json" "${DOTNET_SDK_NUSPEC}")

list(APPEND headers "${CMAKE_BINARY_DIR}/include/coreclr_delegates.h")
list(APPEND dirs "${CMAKE_BINARY_DIR}/include")

set(SHARED_HEADERS ${headers} PARENT_SCOPE)
set(SHARED_INCLUDE_DIRS ${dirs} PARENT_SCOPE)
