#include "config.h"
#include "common.h"

#if RUNTIME_CORECLR && !PLATFORM_WIN

#include "managed_exports.h"
#include "bridge.h"
#include "utils.h"

#include <filesystem>
#include <iostream>

#if PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include <cassert>
#include <dnne.h>
#include <stdio.h>
#include <dlfcn.h>

namespace fs = std::filesystem;

#define HOST_PROPERTY_RUNTIME_CONTRACT CSH_STR("HOST_RUNTIME_CONTRACT")
#define HOST_PROPERTY_APP_PATHS CSH_STR("APP_PATHS")
#define HOST_PROPERTY_BUNDLE_PROBE CSH_STR("BUNDLE_PROBE")
#define HOST_PROPERTY_ENTRY_ASSEMBLY_NAME CSH_STR("ENTRY_ASSEMBLY_NAME")
#define HOST_PROPERTY_HOSTPOLICY_EMBEDDED CSH_STR("HOSTPOLICY_EMBEDDED")
#define HOST_PROPERTY_NATIVE_DLL_SEARCH_DIRECTORIES CSH_STR("NATIVE_DLL_SEARCH_DIRECTORIES")
#define HOST_PROPERTY_PINVOKE_OVERRIDE CSH_STR("PINVOKE_OVERRIDE")
#define HOST_PROPERTY_PLATFORM_RESOURCE_ROOTS CSH_STR"PLATFORM_RESOURCE_ROOTS")
#define HOST_PROPERTY_TRUSTED_PLATFORM_ASSEMBLIES CSH_STR("TRUSTED_PLATFORM_ASSEMBLIES")
#define HOST_PROPERTY_RUNTIME_IDENTIFIER CSH_STR("RUNTIME_IDENTIFIER")
#define HOST_PROPERTY_APP_CONTEXT_BASE_DIRECTORY CSH_STR("APP_CONTEXT_BASE_DIRECTORY") // path to where the managed assemblies are (usually at least - RID-specific assemblies will be in subfolders)


constexpr static size_t RUNTIME_IDENTIFIER_INDEX = 0;
constexpr static size_t APP_CONTEXT_BASE_DIRECTORY_INDEX = 1;

constexpr static size_t PROPERTY_COUNT = 2;
using property_array = const char*[PROPERTY_COUNT];
static property_array _property_keys {
        HOST_PROPERTY_RUNTIME_IDENTIFIER,
        HOST_PROPERTY_APP_CONTEXT_BASE_DIRECTORY,
};

static property_array _property_values  {
	runtime_identifier,
	nullptr,
};

void *coreclr_handle = NULL;
unsigned int coreclr_domainId = 0;

static void* load_symbol(void* handle, const std::string& name)
{
    assert(!name.empty());

    void* ret;
    if (!handle) {
        ret = dlsym(RTLD_SELF, name.c_str());
    } else {
        ret = dlsym(handle, name.c_str());
    }
    assert(ret != nullptr);
    return ret;
}

#ifdef __cplusplus
extern "C" {
#endif

void *get_fast_callable_managed_function(
        const char_t *dotnet_type,
        const char_t *dotnet_type_method) {
    std::string typeAndAssembly{dotnet_type};
    auto pos = typeAndAssembly.find(", ");
    std::string type = typeAndAssembly.substr(0, pos);
    std::string assembly = typeAndAssembly.substr(pos + 2);

    void *del = NULL;
    int rv = coreclr_create_delegate(coreclr_handle, coreclr_domainId, assembly.c_str(), type.c_str(),
                                     dotnet_type_method, &del);
    return del;
}

#ifdef __cplusplus
}
#endif

char *strdup_printf (int* len, const char *msg, ...)
{
	// COOP: no managed memory access: any mode
	va_list args;
	char *formatted = NULL;

	va_start (args, msg);
	*len = vasprintf (&formatted, msg, args);
	va_end (args);

	return formatted;
}

 void* pinvoke_override (const char *libraryName, const char *entrypointName)
 {
 	void* symbol = nullptr;
 	if (!strcmp(libraryName, "__Internal") ||
 		//!strcmp(libraryName, "SDL2") ||
 		!strcmp(libraryName, "cimgui")) {
 		symbol = load_symbol(nullptr, entrypointName);
 	}
 	return symbol;
 }

int load_managed_runtime()
{
	fs::path assemblyPath = ".";
	auto assemblyName = ASSEMBLYNAME ".dll";
	fs::path startAssembly = assemblyPath / assemblyName;

	auto basePath = normalizePath(".");

	auto runtimePath = fs::path{normalizePath("sdk")};
	
	// _property_values[APP_CONTEXT_BASE_DIRECTORY_INDEX] = runtimePath.c_str();

	std::string paths;
	for (auto const& dir_entry : fs::directory_iterator{runtimePath})
	{
		if (dir_entry.is_directory())
			continue;

		if (dir_entry.path().extension() != ".dll")
			continue;

		if (dir_entry.path().stem() == ASSEMBLYNAME)
			continue;

		if (paths.empty())
			paths = dir_entry.path().string();
		else
		{
			paths += ":";
			paths += dir_entry.path().string();
		}
	}

    int len;
	char *pinvoke_override_ptr = strdup_printf (&len, "%p", &pinvoke_override);

	const char *propertyKeys[] = {
            HOST_PROPERTY_APP_CONTEXT_BASE_DIRECTORY, // path to where the managed assemblies are (usually at least - RID-specific assemblies will be in subfolders)
            HOST_PROPERTY_APP_PATHS,
            HOST_PROPERTY_TRUSTED_PLATFORM_ASSEMBLIES,
            HOST_PROPERTY_RUNTIME_IDENTIFIER,
            HOST_PROPERTY_PINVOKE_OVERRIDE
    };

	const char *propertyValues[] = {
		basePath.c_str(),
		basePath.c_str(),
		paths.c_str(),
		runtime_identifier,
        pinvoke_override_ptr,
	};

    int rv = coreclr_initialize (
		basePath.c_str(),
		ASSEMBLYNAME,
		sizeof(propertyKeys) / sizeof(char*),
		propertyKeys,
		propertyValues,
		&coreclr_handle,
		&coreclr_domainId
	);

    return rv;
}

int register_icall(const char* name, const void* fnptr)
{
	return 0;
}

#endif
