#include "config.h"
#include "common.h"

#if RUNTIME_CORECLR && PLATFORM_WIN

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

#if PLATFORM_WIN
static void* load_library(const std::wstring& path)
{
    assert(!path.empty());
    HMODULE h = LoadLibraryW(path.c_str());
    assert(h != NULL);
    return (void*)h;
}
static void* load_symbol(void* h, const std::string& name)
{
    assert(!name.empty());

	if (!h)
	{
		h = GetModuleHandle(nullptr);
	}

    void* f = GetProcAddress((HMODULE)h, name.c_str());
    assert(f != nullptr);
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
		coreclr_initialize_fptr = (coreclr_initialize_ptr)load_symbol(coreclr_libhandle, "coreclr_initialize");
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
		coreclr_create_delegate_fptr = (coreclr_create_delegate_ptr)load_symbol(coreclr_libhandle, "coreclr_create_delegate");
	}

	if (coreclr_create_delegate_fptr == nullptr)
	{
		std::cout << "Error loading coreclr_create_delegate" << std::endl;
		return -1;
	}

	std::cout << "coreclr_create_delegate " << coreclr_create_delegate_fptr << std::endl;

	return coreclr_create_delegate_fptr(hostHandle, domainId, entryPointAssemblyName, entryPointTypeName, entryPointMethodName, delegate);
}

#else
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
#endif

#ifdef __cplusplus
extern "C" {
#endif

void* get_fast_callable_managed_function(
   const char_t* dotnet_type,
   const char_t* dotnet_type_method)
{
	const std::wstring typeAndAssembly{dotnet_type};
	const size_t pos = typeAndAssembly.find(L", ");
	const std::wstring type = typeAndAssembly.substr(0, pos);
	const std::wstring assembly = typeAndAssembly.substr(pos + 2);

	void *del = nullptr;
	int rv = coreclr_create_delegate (coreclr_handle, coreclr_domainId, UnicodeToString(assembly).c_str(), UnicodeToString(type).c_str(), UnicodeToString(dotnet_type_method).c_str(), &del);
	return del;
}

#ifdef __cplusplus
}
#endif

#if _WIN32 && 0

int load_managed_runtime()
{
	int ret = try_preload_runtime();
	RETURN_FAIL_IF_FALSE(ret == 0, "try_preload_runtime failed\n");
	preload_runtime();
	return 0;
}

#else

/*
 * vscprintf:
 * MSVC implements this as _vscprintf, thus we just 'symlink' it here
 * GNU-C-compatible compilers do not implement this, thus we implement it here
 */
#ifdef _MSC_VER
#define vscprintf _vscprintf
#endif

#ifdef __GNUC__
int vscprintf(const char *format, va_list ap)
{
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int retval = vsnprintf(NULL, 0, format, ap_copy);
    va_end(ap_copy);
    return retval;
}
#endif

#ifdef _MSC_VER
int vasprintf(char **strp, const char *format, va_list ap)
{
	const int len = vscprintf(format, ap);
    if (len == -1)
        return -1;
    char *str = (char*)malloc((size_t)len + 1);
    if (!str)
        return -1;
	const int retval = vsnprintf(str, len + 1, format, ap);
    if (retval == -1) {
        free(str);
        return -1;
    }
    *strp = str;
    return retval;
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
 		!strcmp(libraryName, "SDL2") ||
 		!strcmp(libraryName, "cimgui")) {
 		symbol = load_symbol(nullptr, entrypointName);
 	}
 	return symbol;
 }


char_t* pinvoke_override_ptr;

const char_t *propertyKeys[] = {
	HOST_PROPERTY_APP_CONTEXT_BASE_DIRECTORY,
	HOST_PROPERTY_APP_PATHS,
	HOST_PROPERTY_TRUSTED_PLATFORM_ASSEMBLIES,
	HOST_PROPERTY_RUNTIME_IDENTIFIER,
	HOST_PROPERTY_PINVOKE_OVERRIDE,
	HOST_PROPERTY_NATIVE_DLL_SEARCH_DIRECTORIES,
};
char_t *propertyValues[] = {
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr
};


char_t* copy(WSTRING str) {
	char_t* ret = new char_t[str.size()+1];
	std::copy(str.begin(), str.end(), ret);
	ret[str.size()] = L'\0';
	return ret;
}

//char* copy(std::string str) {
//	char* ret = new char[str.size()+1];
//	std::copy(str.begin(), str.end(), ret);
//	ret[str.size()] = '\0';
//	return ret;
//}

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
	char *strptr = strdup_printf (&len, "0x%pZ", &pinvoke_override);
	pinvoke_override_ptr = StringToUnicode(strptr, len);
	//pinvoke_override_ptr = copy(str);
	//printf("%ls\n", pinvoke_override_ptr);


	//const char *propertyKeys[] = {
	//	"APP_CONTEXT_BASE_DIRECTORY", // path to where the managed assemblies are (usually at least - RID-specific assemblies will be in subfolders)
	//	"APP_PATHS",
	//	"TRUSTED_PLATFORM_ASSEMBLIES",
	//	"RUNTIME_IDENTIFIER",
	//	"PINVOKE_OVERRIDE"
	//};
	//const char *propertyValues[] = {
	//	basePath.c_str(),
	//	basePath.c_str(),
	//	paths.c_str(),
	//	runtime_identifier,
	//	pinvokeOverride,
	//};

	propertyValues[0] = copy(StringToUnicode(basePath));
	propertyValues[1] = copy(StringToUnicode(basePath));
	propertyValues[2] = copy(StringToUnicode(paths));
	propertyValues[5] = copy(StringToUnicode(basePath));

 //   int rv = coreclr_initialize (
	//	basePath.c_str(),
	//	ASSEMBLYNAME,
	//	sizeof(propertyKeys) / sizeof(char*),
	//	propertyKeys,
	//	propertyValues,
	//	&coreclr_handle,
	//	&coreclr_domainId
	//);

 //   return rv;

	return 0;
}

int register_icall(const char* name, const void* fnptr)
{
	return 0;
}
#endif

#endif
