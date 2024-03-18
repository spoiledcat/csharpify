find_program(DOTNET "dotnet")
if ("${DOTNET}" STREQUAL "DOTNET-NOTFOUND")
	message(FATAL_ERROR "dotnet could not be found!")
else()
	execute_process(COMMAND "${DOTNET}" --version
									OUTPUT_VARIABLE DOTNET_VERSION
									OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

set(ORIGINAL_PATH $ENV{PATH})
set(DOTNET_VERSION ${DOTNET_VERSION})

# Find dotnet versions
cmake_path(GET DOTNET PARENT_PATH DOTNET_PATH)
cmake_path(APPEND DOTNET_PATH "sdk/${DOTNET_VERSION}" OUTPUT_VARIABLE DOTNET_PATH)
cmake_path(APPEND DOTNET_PATH "Microsoft.NETCoreSdk.BundledVersions.props" OUTPUT_VARIABLE VERSIONS_PROPS)
file(READ "${VERSIONS_PROPS}" VersionProps_CONTENT)
string(REGEX MATCH "<BundledNETCoreAppPackageVersion>(.*)</BundledNETCoreAppPackageVersion>" DOTNET_SDK_VERSION "${VersionProps_CONTENT}")
set(DOTNET_SDK_VERSION "${CMAKE_MATCH_1}")
message(STATUS "Using ${DOTNET} version ${DOTNET_VERSION} sdk ${DOTNET_SDK_VERSION}")

string(REGEX MATCH "([0-9]+)\.([0-9]+)\\.([0-9]+)" VERSION_PARTS "${DOTNET_VERSION}")
set(DOTNET_MAJOR "${CMAKE_MATCH_1}")
set(DOTNET_MINOR "${CMAKE_MATCH_2}")
set(DOTNET_PATCH "${CMAKE_MATCH_3}")
message(STATUS ".NET SDK:${DOTNET_SDK_VERSION}")
message(STATUS ".NET Version:${DOTNET_MAJOR}.${DOTNET_MINOR}.${DOTNET_PATCH}")

cmake_path(APPEND CMAKE_SOURCE_DIR ".packages" OUTPUT_VARIABLE DOTNET_PACKAGES_PATH)

set(RID "${DOTNET_PLATFORM}-${DOTNET_ARCH}" CACHE STRING ".NET Runtime Identifier" FORCE)

# this is useful on all platforms for the coreclr definitions
set(DOTNET_APPHOST_PATH
	"${DOTNET_PACKAGES_PATH}/microsoft.netcore.app.host.win-x64/${DOTNET_SDK_VERSION}/runtimes/win-x64/native"
	CACHE STRING ".NET App Host" FORCE
)

set(DOTNET_SDK_PATH_ROOT
	"${DOTNET_PACKAGES_PATH}/microsoft.netcore.app.runtime.${runtimesuffix}${RID}/${DOTNET_SDK_VERSION}/runtimes/${RID}"
	CACHE STRING ".NET SDK root" FORCE
)

set(DOTNET_SDK_NUSPEC
	"${DOTNET_PACKAGES_PATH}/microsoft.netcore.app.runtime.${runtimesuffix}${RID}/${DOTNET_SDK_VERSION}/microsoft.netcore.app.runtime.${runtimesuffix}${RID}.nuspec"
	CACHE STRING ".NET SDK nuspec marker" FORCE
)

set(DOTNET_SDK_PATH "${DOTNET_SDK_PATH_ROOT}/lib/net${DOTNET_MAJOR}.${DOTNET_MINOR}" CACHE STRING ".NET SDK path" FORCE)
set(DOTNET_LIBRARY_PATH "${DOTNET_SDK_PATH_ROOT}/native" CACHE STRING ".NET lib path for linking" FORCE)

if(NOT DOTNET_INCLUDE_DIRS)
	set(DOTNET_INCLUDE_DIRS "")
endif()

if(RUNTIME_MONO)
	list(APPEND DOTNET_INCLUDE_DIRS "${DOTNET_LIBRARY_PATH}/include/mono-2.0/")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

if(RUNTIME_CORECLR AND (NOT DOTNET_PLATFORM STREQUAL "win"))
	find_library(CORECLR coreclr PATHS "${DOTNET_LIBRARY_PATH}")
elseif(RUNTIME_MONO)
	find_library(CORECLR coreclr PATHS "${DOTNET_LIBRARY_PATH}")
endif()

message(STATUS ".NET RID: ${RID}")
message(STATUS ".NET Runtime: ${DOTNET_RUNTIME}")
message(STATUS ".NET SDK: ${DOTNET_SDK_PATH_ROOT}")
