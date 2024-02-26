project(csharpify_managed)

cmake_path(APPEND PROJECT_SOURCE_DIR "managed.csproj" OUTPUT_VARIABLE CSPROJ)
cmake_path(APPEND PROJECT_BINARY_DIR "publish" OUTPUT_VARIABLE csharpify_managed_PUBLISH_DIR)
cmake_path(APPEND PROJECT_SOURCE_DIR "generated" OUTPUT_VARIABLE generated_INCLUDE_DIR)
cmake_path(ABSOLUTE_PATH generated_INCLUDE_DIR BASE_DIRECTORY "${CMAKE_SOURCE_DIR}" NORMALIZE OUTPUT_VARIABLE generated_INCLUDE_DIR)
set(generated_INCLUDE_DIR "${generated_INCLUDE_DIR}" PARENT_SCOPE)
set(csharpify_managed_PUBLISH_DIR "${csharpify_managed_PUBLISH_DIR}" PARENT_SCOPE)
cmake_path(GET CSPROJ STEM ASSEMBLYNAME)
set(ASSEMBLYNAME "${ASSEMBLYNAME}" CACHE STRING "Assembly name")

set(build_outputs
  managed.dll
  managed.pdb
)

add_custom_command(
  OUTPUT ${build_outputs}
  COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} build \"${CSPROJ}\" --nologo -c $<CONFIG> -o \"${PROJECT_BINARY_DIR}\"
  BYPRODUCTS "${generated_INCLUDE_DIR}/dnne.h" "${generated_INCLUDE_DIR}/exports.h" "${generated_INCLUDE_DIR}/exports.g.c"
)

add_custom_command(
  OUTPUT publish/managed.dll
  COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} publish \"${CSPROJ}\" --sc -r ${RID} --nologo -c $<CONFIG> -o \"${csharpify_managed_PUBLISH_DIR}\"
  DEPENDS managed.dll
)

add_custom_target(csharpify_managed
  DEPENDS publish/managed.dll
)


# Bundle everything into a runnable directory
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_custom_target(copy_sdk
  COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${csharpify_managed_PUBLISH_DIR}" "${CMAKE_BINARY_DIR}/bin/sdk"
  DEPENDS publish/managed.dll
)

foreach(thing ${build_outputs})
list(APPEND bundle_outputs "${CMAKE_BINARY_DIR}/bin/${thing}") 
endforeach() 

add_custom_command(
  OUTPUT ${bundle_outputs}
  COMMAND ${CMAKE_COMMAND} -E copy -t "${CMAKE_BINARY_DIR}/bin/" ${build_outputs}
  DEPENDS managed.dll
)

add_custom_target(csharpify_managed_bundled DEPENDS copy_sdk ${bundle_outputs})
