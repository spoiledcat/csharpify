project(csharpify_managed)

cmake_path(APPEND PROJECT_SOURCE_DIR "managed.csproj" OUTPUT_VARIABLE CSPROJ)
cmake_path(APPEND PROJECT_BINARY_DIR "publish" OUTPUT_VARIABLE csharpify_managed_PUBLISH_DIR)
cmake_path(APPEND PROJECT_SOURCE_DIR "generated" OUTPUT_VARIABLE generated_INCLUDE_DIR)
cmake_path(ABSOLUTE_PATH generated_INCLUDE_DIR BASE_DIRECTORY "${CMAKE_SOURCE_DIR}" NORMALIZE OUTPUT_VARIABLE generated_INCLUDE_DIR)
set(generated_INCLUDE_DIR "${generated_INCLUDE_DIR}" PARENT_SCOPE)
set(csharpify_managed_PUBLISH_DIR "${csharpify_managed_PUBLISH_DIR}" PARENT_SCOPE)
cmake_path(GET CSPROJ STEM ASSEMBLYNAME)
set(ASSEMBLYNAME "${ASSEMBLYNAME}" CACHE STRING "Assembly name")

list(APPEND build_outputs
  managed.dll
  managed.pdb
)

if(DOTNET_PLATFORM STREQUAL "win" AND DOTNET_RUNTIME STREQUAL "coreclr")
  list(APPEND build_outputs managed.runtimeconfig.json)
endif()

add_custom_command(
  OUTPUT ${build_outputs} "${generated_INCLUDE_DIR}/exports.g.c"
  COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} build \"${CSPROJ}\" --nologo -c $<CONFIG> -o \"${PROJECT_BINARY_DIR}\" /p:UseMonoRuntime=$<IF:$<BOOL:${RUNTIME_MONO}>,true,false>
  BYPRODUCTS "${generated_INCLUDE_DIR}/dnne.h" "${generated_INCLUDE_DIR}/exports.h" "${generated_INCLUDE_DIR}/platform.c"
  DEPENDS "Directory.Build.props" "Directory.Build.targets" "main.cs" "managed.csproj"
)

add_custom_target(build DEPENDS ${build_outputs} "${generated_INCLUDE_DIR}/exports.g.c")

set(publish_args "")
set(dotnet_publish_dir "${csharpify_managed_PUBLISH_DIR}/publish")
set(dotnet_sc_dir "${csharpify_managed_PUBLISH_DIR}/sc")
if(DOTNET_PLATFORM STREQUAL "win")
  set(bin_dir "${CMAKE_BINARY_DIR}/native/Debug")
else()
  set(bin_dir "${CMAKE_BINARY_DIR}/native")
endif()


add_custom_command(
  OUTPUT "${dotnet_publish_dir}/managed.dll"
  COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} publish \"${CSPROJ}\" ${publish_args} --nologo -c $<CONFIG> -o \"${dotnet_publish_dir}\"  /p:UseMonoRuntime=$<IF:$<BOOL:${RUNTIME_MONO}>,true,false>
  DEPENDS ${build_outputs} "${generated_INCLUDE_DIR}/exports.g.c"
)

# Bundle everything into a runnable directory
file(MAKE_DIRECTORY "${bin_dir}")

add_custom_command(
  OUTPUT "${bin_dir}/managed.dll"
  COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} publish \"${CSPROJ}\" ${publish_args} --nologo -c $<CONFIG> -o \"${bin_dir}\"  /p:UseMonoRuntime=$<IF:$<BOOL:${RUNTIME_MONO}>,true,false>
  DEPENDS ${build_outputs} "${generated_INCLUDE_DIR}/exports.g.c"
)

add_custom_target(managed_publish DEPENDS "${dotnet_publish_dir}/managed.dll" "${bin_dir}/managed.dll")

add_custom_command(
  OUTPUT "${dotnet_sc_dir}/managed.dll"
  COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} publish \"${CSPROJ}\" ${publish_args} --sc -r ${RID} --nologo -c $<CONFIG> -o \"${dotnet_sc_dir}\"  /p:UseMonoRuntime=$<IF:$<BOOL:${RUNTIME_MONO}>,true,false>
  DEPENDS ${build_outputs} "${generated_INCLUDE_DIR}/exports.g.c"
)

add_custom_target(managed_sc DEPENDS "${dotnet_sc_dir}/managed.dll")

if(NOT(DOTNET_PLATFORM STREQUAL "win" AND DOTNET_RUNTIME STREQUAL "coreclr"))
  add_custom_target(copy_sdk
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${dotnet_sc_dir}" "${bin_dir}/sdk"
    DEPENDS "${dotnet_sc_dir}/System.dll"
  )
list(APPEND sdk copy_sdk)
endif()

add_custom_target(csharpify_managed ALL DEPENDS "${dotnet_publish_dir}/managed.dll" "${dotnet_sc_dir}/managed.dll" "${bin_dir}/managed.dll")
add_custom_target(csharpify_managed_bundled ALL DEPENDS ${sdk})
