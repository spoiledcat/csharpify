project(csharpify_managed)

cmake_path(APPEND PROJECT_SOURCE_DIR "managed.csproj" OUTPUT_VARIABLE CSPROJ)
cmake_path(APPEND PROJECT_BINARY_DIR "publish" OUTPUT_VARIABLE csharpify_managed_PUBLISH_DIR)
cmake_path(APPEND PROJECT_SOURCE_DIR "generated" OUTPUT_VARIABLE generated_INCLUDE_DIR)
cmake_path(ABSOLUTE_PATH generated_INCLUDE_DIR BASE_DIRECTORY "${CMAKE_SOURCE_DIR}" NORMALIZE OUTPUT_VARIABLE generated_INCLUDE_DIR)
set(generated_INCLUDE_DIR "${generated_INCLUDE_DIR}" PARENT_SCOPE)
set(csharpify_managed_PUBLISH_DIR "${csharpify_managed_PUBLISH_DIR}" PARENT_SCOPE)
cmake_path(GET CSPROJ STEM ASSEMBLYNAME)
set(ASSEMBLYNAME "${ASSEMBLYNAME}" CACHE STRING "Assembly name")

set(publish_args "")
set(dotnet_publish_dir "${csharpify_managed_PUBLISH_DIR}/publish")
set(dotnet_sc_dir "${csharpify_managed_PUBLISH_DIR}/sc")
set(bin_dir "${CMAKE_BINARY_DIR}/native")
set(bundle_dir "${CMAKE_BINARY_DIR}/bin")

configure_file("${CMAKE_CURRENT_LIST_DIR}/Directory.Build.props.in" "${CMAKE_CURRENT_LIST_DIR}/Directory.Build.props")

list(APPEND build_outputs ${ASSEMBLYNAME}.dll ${ASSEMBLYNAME}.pdb)
if(DOTNET_PLATFORM STREQUAL "win" AND DOTNET_RUNTIME STREQUAL "coreclr")
  list(APPEND build_outputs ${ASSEMBLYNAME}.runtimeconfig.json)
endif()

add_custom_command(
  OUTPUT ${build_outputs} "${generated_INCLUDE_DIR}/exports.g.c"
  COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} build \"${CSPROJ}\" --nologo -c $<CONFIG> -o \"${PROJECT_BINARY_DIR}\" /p:UseMonoRuntime=$<IF:$<BOOL:${RUNTIME_MONO}>,true,false>
  BYPRODUCTS "${generated_INCLUDE_DIR}/dnne.h" "${generated_INCLUDE_DIR}/exports.h"
  DEPENDS "Directory.Build.props" "Directory.Build.targets" "main.cs" "managed.csproj"
)

add_custom_target(build DEPENDS ${build_outputs} "${generated_INCLUDE_DIR}/exports.g.c")

add_custom_command(
  OUTPUT "${dotnet_publish_dir}/${ASSEMBLYNAME}.dll"
  COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} publish \"${CSPROJ}\" ${publish_args} --nologo -c $<CONFIG> -o \"${dotnet_publish_dir}\"  /p:UseMonoRuntime=$<IF:$<BOOL:${RUNTIME_MONO}>,true,false>
  DEPENDS "Directory.Build.props" "Directory.Build.targets" "main.cs" "managed.csproj"
)

# Bundle everything into a runnable directory
file(MAKE_DIRECTORY "${bin_dir}")

add_custom_command(
  OUTPUT "${bin_dir}/${ASSEMBLYNAME}.dll"
  COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} publish \"${CSPROJ}\" ${publish_args} --nologo -c $<CONFIG> -o \"${bin_dir}\"  /p:UseMonoRuntime=$<IF:$<BOOL:${RUNTIME_MONO}>,true,false>
  DEPENDS "Directory.Build.props" "Directory.Build.targets" "main.cs" "managed.csproj"
)

add_custom_target(managed_publish DEPENDS "${dotnet_publish_dir}/${ASSEMBLYNAME}.dll" "${bin_dir}/${ASSEMBLYNAME}.dll")

add_custom_command(
  OUTPUT "${dotnet_sc_dir}/${ASSEMBLYNAME}.dll" "${dotnet_sc_dir}/System.dll"
  COMMAND ${CMAKE_COMMAND} -E env "PATH=${ORIGINAL_PATH}" ${DOTNET} publish \"${CSPROJ}\" ${publish_args} --sc -r ${RID} --nologo -c $<CONFIG> -o \"${dotnet_sc_dir}\"  /p:UseMonoRuntime=$<IF:$<BOOL:${RUNTIME_MONO}>,true,false>
  DEPENDS "Directory.Build.props" "Directory.Build.targets" "main.cs" "managed.csproj"
)

add_custom_target(managed_sc DEPENDS "${dotnet_sc_dir}/${ASSEMBLYNAME}.dll")

if(NOT(DOTNET_PLATFORM STREQUAL "win" AND DOTNET_RUNTIME STREQUAL "coreclr"))
  add_custom_target(copy_sdk
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${dotnet_sc_dir}" "${bin_dir}/sdk"
    DEPENDS "${dotnet_sc_dir}/System.dll"
  )
list(APPEND sdk copy_sdk)
endif()

add_custom_target(csharpify_managed DEPENDS "${dotnet_publish_dir}/${ASSEMBLYNAME}.dll" "${dotnet_sc_dir}/${ASSEMBLYNAME}.dll" "${bin_dir}/${ASSEMBLYNAME}.dll")

# On Windows, we need to copy nethost.dll next to the executable
# and have a shared/Microsoft.NETCore.App/${DOTNET_SDK_VERSION} folder
# with the sdk, which comes from ${DOTNET_SDK_PATH}
# coreclr.dll et al from ${DOTNET_LIBRARY_PATH} go into the shared folder
# and hostfxr.dll goes into its own folder (it must be three levels down
# because it's hardcoded to search for the sdk three levels up, it's stupid)
if(DOTNET_PLATFORM STREQUAL "win" AND DOTNET_RUNTIME STREQUAL "coreclr")
  list(APPEND build_outputs
    "${DOTNET_APPHOST_PATH}/nethost.dll"
  )

  foreach(thing ${build_outputs})
    cmake_path(GET thing FILENAME filename)
    list(APPEND bundle_outputs "${bundle_dir}/${CMAKE_BUILD_TYPE}/${filename}")
  endforeach()

  list(APPEND bundle_outputs
    "${bundle_dir}/$<CONFIG>/${ASSEMBLYNAME}.dll"
    "${bundle_dir}/$<CONFIG>/shared/Microsoft.NETCore.App/${DOTNET_SDK_VERSION}/Microsoft.NETCore.App.runtimeconfig.json"
    "${bundle_dir}/$<CONFIG>/shared/Microsoft.NETCore.App/${DOTNET_SDK_VERSION}/hostpolicy.dll"
    "${bundle_dir}/$<CONFIG>/shared/Microsoft.NETCore.App/${DOTNET_SDK_VERSION}/coreclr.dll"
    "${bundle_dir}/$<CONFIG>/shared/host/fxr/hostfxr.dll"
  )

  # Bundle everything into a runnable directory
  add_custom_command(
    OUTPUT  ${bundle_outputs} 
    COMMAND ${CMAKE_COMMAND} -E make_directory "${bundle_dir}/$<CONFIG>"
    COMMAND ${CMAKE_COMMAND} -E copy -t "${bundle_dir}/$<CONFIG>" ${build_outputs} "${bin_dir}/${ASSEMBLYNAME}.dll"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${bundle_dir}/$<CONFIG>/shared/Microsoft.NETCore.App"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${DOTNET_SDK_PATH}" "${bundle_dir}/$<CONFIG>/shared/Microsoft.NETCore.App/${DOTNET_SDK_VERSION}"
    COMMAND ${CMAKE_COMMAND} -E copy -t "${bundle_dir}/$<CONFIG>/shared/Microsoft.NETCore.App/${DOTNET_SDK_VERSION}" "${DOTNET_LIBRARY_PATH}/hostpolicy.dll"
    COMMAND ${CMAKE_COMMAND} -E copy -t "${bundle_dir}/$<CONFIG>/shared/Microsoft.NETCore.App/${DOTNET_SDK_VERSION}" "${DOTNET_LIBRARY_PATH}/hostpolicy.dll" "${DOTNET_LIBRARY_PATH}/hostfxr.dll"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${bundle_dir}/$<CONFIG>/shared/host/fxr"
    COMMAND ${CMAKE_COMMAND} -E copy -t "${bundle_dir}/$<CONFIG>/shared/host/fxr" "${DOTNET_LIBRARY_PATH}/hostfxr.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${DOTNET_LIBRARY_PATH}" "${bundle_dir}/$<CONFIG>/shared/Microsoft.NETCore.App/${DOTNET_SDK_VERSION}"
    DEPENDS ${build_outputs} "${bin_dir}/${ASSEMBLYNAME}.dll"
  )

  add_custom_target(csharpify_managed_bundled DEPENDS csharpify_managed ${bundle_outputs})
else()
  add_custom_target(csharpify_managed_bundled DEPENDS ${sdk})
  add_dependencies(csharpify_managed csharpify_managed_bundled)
endif()
