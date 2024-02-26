#include "config.h"
#include "main.h"
#include "managed_exports.h"

#include "imgui.h"

#include "coreclr/bridge.h"

#include <filesystem>

namespace fs = std::filesystem; 

static MonoCoreRuntimeProperties monovm_core_properties = {
	.trusted_platform_assemblies = nullptr,
	.app_paths = nullptr,
	.native_dll_search_directories = nullptr,
	.pinvoke_override = nullptr
};

constexpr static char RUNTIME_IDENTIFIER_KEY[] = "RUNTIME_IDENTIFIER";
constexpr static size_t RUNTIME_IDENTIFIER_INDEX = 0;

constexpr static char APP_CONTEXT_BASE_DIRECTORY_KEY[] = "APP_CONTEXT_BASE_DIRECTORY";
constexpr static size_t APP_CONTEXT_BASE_DIRECTORY_INDEX = 1;

constexpr static size_t PROPERTY_COUNT = 2;
using property_array = const char*[PROPERTY_COUNT];
static property_array _property_keys {
	RUNTIME_IDENTIFIER_KEY,
	APP_CONTEXT_BASE_DIRECTORY_KEY,
};

static property_array _property_values  {
	runtime_identifier,
	nullptr,
};


static property_array _property_keys2 {
};

static property_array _property_values2 {
};

void *coreclr_handle = NULL;
unsigned int coreclr_domainId = 0;

#ifdef __cplusplus
extern "C" {
#endif

void* get_fast_callable_managed_function(
    const char_t* dotnet_type,
    const char_t* dotnet_type_method)
{
	std::string typeAndAssembly{dotnet_type};
	auto pos = typeAndAssembly.find(", ");
	std::string type = typeAndAssembly.substr(0, pos);
	std::string assembly = typeAndAssembly.substr(pos + 2);

	void *del = NULL;
	int rv = coreclr_create_delegate (coreclr_handle, coreclr_domainId, assembly.c_str(), type.c_str(), dotnet_type_method, &del);
	return del;
}

#ifdef __cplusplus
}
#endif


void MainLoop()
{
}

int main(int argc, char** argv)
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
			paths = dir_entry.path();
		else
		{
			paths += ":";
			paths += dir_entry.path();
		}
	}

	const char *propertyKeys[] = {
		"APP_CONTEXT_BASE_DIRECTORY", // path to where the managed assemblies are (usually at least - RID-specific assemblies will be in subfolders)
		"APP_PATHS",
		"TRUSTED_PLATFORM_ASSEMBLIES",
		"RUNTIME_IDENTIFIER",
	};
	const char *propertyValues[] = {
		basePath.c_str(),
		basePath.c_str(),
		paths.c_str(),
		runtime_identifier,
	};


	// monovm_initialize_preparsed (
	// 	&monovm_core_properties,
	// 	1,
	// 	const_cast<const char**>(_property_keys),
	// 	const_cast<const char**>(_property_values)
	// );


	int rv = coreclr_initialize (
		runtimePath.c_str(),
		ASSEMBLYNAME,
		4,
		propertyKeys,
		propertyValues,
		&coreclr_handle,
		&coreclr_domainId
	);

	printf("%p\n", rv);
	printf("%p\n", coreclr_handle);
	printf("%d\n", coreclr_domainId);

	if (rv < 0) {
		return -1;
	}

	CallMe();
	CallMe2();

	return imgui_main(argc, argv);
}