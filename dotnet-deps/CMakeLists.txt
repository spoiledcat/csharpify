project(packages)

file(GLOB props "${packages_SOURCE_DIR}/imports/*.props")

add_custom_command(
	OUTPUT "${DOTNET_APPHOST_PATH}/coreclr_delegates.h" "${packages_SOURCE_DIR}/obj/project.assets.json" "${DOTNET_SDK_NUSPEC}"
	COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} "restore"
	WORKING_DIRECTORY "${packages_SOURCE_DIR}"
	DEPENDS packages.csproj ${props}
)

add_custom_target(packages DEPENDS "${DOTNET_APPHOST_PATH}/coreclr_delegates.h" "${packages_SOURCE_DIR}/obj/project.assets.json" "${DOTNET_SDK_NUSPEC}")
