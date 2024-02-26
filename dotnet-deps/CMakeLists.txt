project(packages)

execute_process(
	COMMAND "${DOTNET}" "restore" 
	WORKING_DIRECTORY "${packages_SOURCE_DIR}")

add_custom_target(packages)
