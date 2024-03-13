# use GNU Patch from any platform

if(WIN32)
  # prioritize Git Patch on Windows as other Patches may be very old and incompatible.
  find_package(Git)
  if(Git_FOUND)
    get_filename_component(GIT_DIR ${GIT_EXECUTABLE} DIRECTORY)
    get_filename_component(GIT_DIR ${GIT_DIR} DIRECTORY)
  endif()
endif()

find_program(PATCH
NAMES patch
HINTS ${GIT_DIR}
PATH_SUFFIXES usr/bin
)

if(NOT PATCH)
  message(FATAL_ERROR "Did not find GNU Patch")
endif()

if(out_file)
  execute_process(COMMAND ${PATCH} ${in_file} --input=${patch_file} --output=${out_file} --ignore-whitespace -N -s
    TIMEOUT 15
    OUTPUT_VARIABLE PATCHERR
    ERROR_VARIABLE PATCHERR
    RESULT_VARIABLE ret
  )
else()
  execute_process(COMMAND ${PATCH} ${in_file} --input=${patch_file} --ignore-whitespace -N -s
    TIMEOUT 15
    OUTPUT_VARIABLE PATCHERR
    ERROR_VARIABLE PATCHERR
    RESULT_VARIABLE ret
  )
endif()

if(NOT ret EQUAL 0 AND NOT(PATCHERR MATCHES "Skipping patch") AND NOT(PATCHERR MATCHES "previously applied"))
  message(FATAL_ERROR "Failed to apply patch ${patch_file} to ${in_file} with ${PATCH}")
endif()
