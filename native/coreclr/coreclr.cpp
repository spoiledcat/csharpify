#include "config.h"
#include "bridge.h"
#include "utils.h"

#include <filesystem>
#include <iostream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include <cassert>
#include <dnne.h>

namespace fs = std::filesystem; 


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

#ifdef _WIN32
static void* load_library(const std::wstring& path)
{
    assert(!path.empty());
    HMODULE h = LoadLibraryW(path.c_str());
    assert(h != NULL);
    return (void*)h;
}
static void* get_export(void* h, const std::string& name)
{
    assert(h != NULL && !name.empty());
    void* f = GetProcAddress((HMODULE)h, name.c_str());
    assert(f != NULL);
    return f;
}

static void* coreclr_libhandle;
static coreclr_initialize_ptr coreclr_initialize_fptr;
int CORECLR_CALLING_CONVENTION coreclr_initialize(
            const char* exePath,
            const char* appDomainFriendlyName,
            int propertyCount,
            const char** propertyKeys,
            const char** propertyValues,
            void** hostHandle,
            unsigned int* domainId)
{
	if (coreclr_initialize_fptr == nullptr)
	{
		const auto runtimePath = fs::path{normalizePath("sdk")} / "coreclr.dll";
		coreclr_libhandle = load_library(runtimePath.wstring());
		if (coreclr_libhandle == nullptr)
		{
			std::cout << "Error loading coreclr.dll" << std::endl;
			return -1;
		}
		coreclr_initialize_fptr = (coreclr_initialize_ptr)get_export(coreclr_libhandle, "coreclr_initialize");
	}

	if (coreclr_initialize_fptr == nullptr)
	{
		std::cout << "Error loading coreclr_initialize" << std::endl;
		return -1;
	}

	std::cout << "coreclr_initialize:" << coreclr_initialize_fptr << std::endl;

	std::cout << exePath << ":" << appDomainFriendlyName << ":" << propertyCount << ":" << propertyKeys << ":" << propertyValues << ":" << hostHandle << ":" << domainId << std::endl;

	return coreclr_initialize_fptr(exePath, appDomainFriendlyName, propertyCount, propertyKeys, propertyValues, hostHandle, domainId);
}

static coreclr_create_delegate_ptr coreclr_create_delegate_fptr;
int CORECLR_CALLING_CONVENTION coreclr_create_delegate(
            void* hostHandle,
            unsigned int domainId,
            const char* entryPointAssemblyName,
            const char* entryPointTypeName,
            const char* entryPointMethodName,
            void** delegate)
{
	if (coreclr_create_delegate_fptr == nullptr)
	{
		coreclr_create_delegate_fptr = (coreclr_create_delegate_ptr)get_export(coreclr_libhandle, "coreclr_create_delegate");
	}

	if (coreclr_create_delegate_fptr == nullptr)
	{
		std::cout << "Error loading coreclr_create_delegate" << std::endl;
		return -1;
	}

	std::cout << "coreclr_create_delegate " << coreclr_create_delegate_fptr << std::endl;

	return coreclr_create_delegate_fptr(hostHandle, domainId, entryPointAssemblyName, entryPointTypeName, entryPointMethodName, delegate);
}

#endif


#ifdef _WIN32
//void* get_fast_callable_managed_function(
//    const char_t* dotnet_type,
//    const char_t* dotnet_type_method)
//{
//	const std::wstring typeAndAssembly{dotnet_type};
//	const size_t pos = typeAndAssembly.find(L", ");
//	const std::wstring type = typeAndAssembly.substr(0, pos);
//	const std::wstring assembly = typeAndAssembly.substr(pos + 2);
//
//	void *del = nullptr;
//	int rv = coreclr_create_delegate (coreclr_handle, coreclr_domainId, UnicodeToString(assembly).c_str(), UnicodeToString(type).c_str(), UnicodeToString(dotnet_type_method).c_str(), &del);
//	return del;
//}
#else
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
#endif


#ifdef _WIN32

int load_managed_runtime()
{
	int ret = try_preload_runtime();
	RETURN_FAIL_IF_FALSE(ret == 0, "try_preload_runtime failed\n");
	preload_runtime();
	return 0;
}

#else
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


		int rv = coreclr_initialize (
		runtimePath.string().c_str(),
		ASSEMBLYNAME,
		4,
		propertyKeys,
		propertyValues,
		&coreclr_handle,
		&coreclr_domainId
	);

}
#endif