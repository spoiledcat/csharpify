#include "config.h"
#include "common.h"

#if RUNTIME_CORECLR && PLATFORM_WIN

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "managed_exports.h"
#include "bridge.h"
#include "utils.h"

#include <filesystem>
#include <iostream>

#include <Windows.h>

#include <cassert>
#include <dnne.h>
#include <cstdio>
#include <hostfxr.h>

#define CSH_LOCK_OPEN (0)
#define CSH_LOCK_TAKEN (-1)

#define CSH_MAX_PATH 512
#define CSH_NORETURN __declspec(noreturn)

namespace fs = std::filesystem;
using lock_handle = volatile long;

namespace {
	failure_fn failure_fptr;
	lock_handle prepare_lock = CSH_LOCK_OPEN;

	bool initialized;

	hostfxr_initialize_for_dotnet_command_line_fn init_self_contained_fptr;
	hostfxr_initialize_for_runtime_config_fn init_fptr;
	hostfxr_get_runtime_delegate_fn get_delegate_fptr;
	hostfxr_close_fn close_fptr;
	hostfxr_set_runtime_property_value_fn set_runtime_property_value_fptr;

	load_assembly_and_get_function_pointer_ptr load_assembly_and_get_function_pointer_fptr;
	coreclr_create_delegate_ptr coreclr_create_delegate_fptr;

	std::wstring assemblyPath;
}

CSH_EXTERN_C CSH_EXPORT_API void CSH_CALLTYPE set_failure_callback(failure_fn cb)
{
	failure_fptr = cb;
}

CSH_NORETURN static void noreturn_failure(enum failure_type type, int error_code)
{
	if (failure_fptr)
		failure_fptr(type, error_code);

	// Don't trust anything the user can override.
	abort();
}

#define IF_FAILURE_RETURN_OR_ABORT(type, rc, lock) \
{ \
	if (is_failure(rc)) \
	{ \
		exit_lock(lock); \
		noreturn_failure(type, rc); \
	} \
}

void enter_lock(lock_handle *lock)
{
	while (InterlockedCompareExchange(lock, -1, CSH_LOCK_OPEN) != CSH_LOCK_TAKEN) {
		Sleep(1 /* milliseconds */);
	}
}

void exit_lock(lock_handle *lock)
{
	InterlockedExchange(lock, CSH_LOCK_OPEN);
}

bool is_failure(int rc)
{
	// The CLR hosting API uses the Win32 HRESULT scheme. This means
	// the high order bit indicates an error and S_FALSE (1) can be returned
	// and is _not_ a failure.
	return rc < 0;
}

int get_current_error()
{
	return static_cast<int>(GetLastError());
}

void set_current_error(const int err)
{
	SetLastError(static_cast<DWORD>(err));
}


void *load_library(const std::wstring &path)
{
	assert(!path.empty());
	HMODULE h = LoadLibraryW(path.c_str());
	assert(h != NULL);
	return h;
}

void *load_symbol(void *h, const std::string &name)
{
	assert(!name.empty());

	if (!h) {
		h = GetModuleHandle(nullptr);
	}
	void *f = GetProcAddress(static_cast<HMODULE>(h), name.c_str());
	assert(f != nullptr);
	return f;
}

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
	auto str = static_cast<char *>(malloc(static_cast<size_t>(len) + 1));
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

char_t *copy(WSTRING str)
{
	const auto ret = new char_t[str.size() + 1];
	std::copy(str.begin(), str.end(), ret);
	ret[str.size()] = L'\0';
	return ret;
}

char *strdup_printf(int *len, const char *msg, ...)
{
	// COOP: no managed memory access: any mode
	va_list args;
	char *formatted = nullptr;

	va_start(args, msg);
	*len = vasprintf(&formatted, msg, args);
	va_end(args);

	return formatted;
}

void *pinvoke_override(const char *libraryName, const char *entrypointName)
{
	void *symbol = nullptr;
	if (!strcmp(libraryName, "__Internal") ||
		!strcmp(libraryName, "SDL2") ||
		!strcmp(libraryName, "cimgui")) {
		symbol = load_symbol(nullptr, entrypointName);
	}
	return symbol;
}

int load_hostfxr()
{
	const auto path = fs::path(assemblyPath).parent_path().append("shared/host/fxr/hostfxr.dll");

	// Load hostfxr and get desired exports.
	void *lib = load_library(path);

	init_self_contained_fptr = (hostfxr_initialize_for_dotnet_command_line_fn)load_symbol(lib, "hostfxr_initialize_for_dotnet_command_line");
	init_fptr = (hostfxr_initialize_for_runtime_config_fn)load_symbol(lib, "hostfxr_initialize_for_runtime_config");
	get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)load_symbol(lib, "hostfxr_get_runtime_delegate");
	close_fptr = (hostfxr_close_fn)load_symbol(lib, "hostfxr_close");
	set_runtime_property_value_fptr = (hostfxr_set_runtime_property_value_fn)load_symbol(lib, "hostfxr_set_runtime_property_value");

	assert(init_self_contained_fptr && init_fptr && get_delegate_fptr && close_fptr);
	return 0;
}

int init_dotnet()
{
	// Self-contained scenario support relies upon the application scenario
	// entry-point. The logic here is to trick the hosting API into initializing as an application
	// but call the "load assembly and get delegate" instead of "run main". This has impact
	// on the TPA make-up and hence assembly loading in general since the TPA populates the default ALC.
	const char_t *configPath = assemblyPath.c_str();

	// Load .NET runtime
	hostfxr_handle ctx = nullptr;
	int rc = init_self_contained_fptr(1, &configPath, nullptr, &ctx);
	if (is_failure(rc)) {
		close_fptr(ctx);
		return rc;
	}

	int len;
	const char* strptr = strdup_printf(&len, "0x%pZ", &pinvoke_override);
	const char_t* pinvoke = StringToUnicode(strptr, len);
	set_runtime_property_value_fptr(ctx, CSH_STR("PINVOKE_OVERRIDE"), pinvoke);

	void* ptr;
	// Get the load assembly function pointer
	rc = get_delegate_fptr(ctx, hdt_load_assembly_and_get_function_pointer, &ptr);
	if (is_failure(rc)) {
		close_fptr(ctx);
		return rc;
	}

	load_assembly_and_get_function_pointer_fptr = (load_assembly_and_get_function_pointer_ptr)ptr;
	return DNNE_SUCCESS;
}

int load_managed_runtime()
{
	// Lock and check if the needed export was already acquired.
	enter_lock(&prepare_lock);
	if (!initialized) {
		const auto assemblyName = ASSEMBLYNAME ".dll";
		fs::path startAssembly = fs::path(".") / assemblyName;
		assemblyPath = normalizePathW(startAssembly);

		// Load HostFxr and get exported hosting functions.
		int rc = load_hostfxr();
		IF_FAILURE_RETURN_OR_ABORT(failure_load_runtime, rc, &prepare_lock);

		// Initialize and start the runtime.
		rc = init_dotnet();
		IF_FAILURE_RETURN_OR_ABORT(failure_load_runtime, rc, &prepare_lock);

		assert(load_assembly_and_get_function_pointer_fptr != nullptr);
		initialized = true;
	}
	exit_lock(&prepare_lock);

	return 0;
}

int register_icall(const char *name, const void *fnptr)
{
	return 0;
}

void *get_callable_managed_function(const char_t *dotnetType, const char_t *dotnetTypeMethod,
									const char_t *dotnetDelegateType)
{
	assert(dotnetType && dotnetTypeMethod);

	// Store the current error state to reset it when
	// we exit this function. This being done because this
	// API is an implementation detail of the export but
	// can result in side-effects during export resolution.
	const int currError = get_current_error();
	int rc;
	// Check if the runtime has already been prepared.
	if (!initialized) {
		rc = load_managed_runtime();
		if (is_failure(rc)) {
			noreturn_failure(failure_load_export, rc);
		}
	}

	// Function pointer to managed function
	void *func = nullptr;
	rc = load_assembly_and_get_function_pointer_fptr(
		assemblyPath.c_str(),
		dotnetType,
		dotnetTypeMethod,
		dotnetDelegateType,
		nullptr,
		&func);

	if (is_failure(rc))
		noreturn_failure(failure_load_export, rc);

	// Now that the export has been resolved, reset
	// the error state to hide this implementation detail.
	set_current_error(currError);
	return func;
}

CSH_EXTERN_C
void *get_fast_callable_managed_function(const char_t *dotnetType,const char_t *dotnetTypeMethod)
{
	return get_callable_managed_function(dotnetType, dotnetTypeMethod, (const char_t *)-1);
}
#endif
