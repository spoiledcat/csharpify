#include "config.h"
#include "common.h"

#if RUNTIME_MONO

#if PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif

#include "managed_exports.h"
#include "bridge.h"
#include "utils.h"

#include <mono/metadata/mono-debug.h>
#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>
#include <mono/utils/mono-error.h>

#include <filesystem>
#include <cassert>

#if PLATFORM_WIN
#include <iostream>
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;


namespace
{
    MonoAssembly *entry_assembly = nullptr;
    failure_fn failure_fptr;

    bool initialized;

    load_assembly_and_get_function_pointer_ptr load_assembly_and_get_function_pointer_fptr;
    coreclr_create_delegate_ptr coreclr_create_delegate_fptr;
} // namespace

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

#if PLATFORM_WIN
char_t * copy(WSTRING str)
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

char_t *to_char_t(WSTRING str)
{
    const auto ret = new char_t[str.size() + 1];
    std::copy(str.begin(), str.end(), ret);
    ret[str.size()] = L'\0';
    return ret;
}
#endif

MonoAssembly*
assembly_preload_hook(MonoAssemblyName* aname, char** assemblies_path, void* user_data) {
    char filename[1024];
    char path[1024];

    fs::path name(mono_assembly_name_get_name(aname));
    if (!name.has_extension() || name.extension() != "dll") {
        name.replace_filename(name.string().append(".dll"));
    }

    auto basePath = normalizePath(".");
    auto runtimePath = fs::path{normalizePath("sdk")};

    fs::path possible = fs::path{runtimePath / name};
    if (exists(possible)) {
        return mono_assembly_open(possible.generic_string().c_str(), nullptr);
    }

    possible = fs::path(basePath / name);
    if (exists(possible)) {
        return mono_assembly_open(possible.generic_string().c_str(), nullptr);
    }

    return nullptr;
}

#if !PLATFORM_WIN

static void* load_symbol(void* handle, const std::string &name) {
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
#else
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
#endif


CSH_BEGIN_C
void* pinvoke_override(const char* libraryName, const char* entrypointName) {
    void* symbol = nullptr;
    if (!strcmp(libraryName, "__Internal") ||
        !strcmp(libraryName, "cimgui")) {
        symbol = load_symbol(nullptr, entrypointName);
    }
    return symbol;
}
CSH_END_C


int load_managed_runtime() {
    fs::path assemblyPath = ".";
    auto assemblyName = ASSEMBLYNAME ".dll";
    fs::path startAssembly = assemblyPath / assemblyName;

    auto basePath = normalizePath(".");
    auto runtimePath = fs::path{normalizePath("sdk")};

    const auto path = fs::path(runtimePath).parent_path().append("coreclr.dll");

    const char* propertyKeys[] = {
            HOST_PROPERTY_APP_CONTEXT_BASE_DIRECTORY, // path to where the managed assemblies are (usually at least - RID-specific assemblies will be in subfolders)
            HOST_PROPERTY_RUNTIME_IDENTIFIER
    };

    const char* propertyValues[] = {
            basePath.c_str(),
            runtime_identifier
    };

    monovm_core_properties.pinvoke_override = &pinvoke_override;

    int rv = monovm_initialize_preparsed(
            &monovm_core_properties,
            sizeof(propertyKeys) / sizeof(char*),
            propertyKeys,
            propertyValues
    );

    MonovmRuntimeConfigArguments runtime_config_args;

    mono_debug_init(MONO_DEBUG_FORMAT_MONO);
    mono_install_assembly_preload_hook(assembly_preload_hook, nullptr);

    mono_set_signal_chaining(1);
    mono_set_crash_chaining(1);
    mono_jit_init(ASSEMBLYNAME);

    // The mono initialization resets the preload hooks, so install it again
    mono_install_assembly_preload_hook(assembly_preload_hook, nullptr);

    MonoImageOpenStatus status = MONO_IMAGE_OK;
    entry_assembly = mono_assembly_open(assemblyName, &status);
    initialized = true;
    return rv;
}

#include <mono/metadata/loader.h>

int register_icall(const char* name, const void* fnptr) {
    mono_add_internal_call(name, fnptr);
    return 0;
}

CSH_BEGIN_C

MonoMethod*
mono_marshal_get_managed_wrapper(MonoMethod* method, MonoClass* delegate_klass, MonoGCHandle target_handle,
                                 MonoError* error);
void*
mono_compile_method_checked(MonoMethod* method, MonoError* error);
#define is_ok(error) ((error).error_code == MONO_ERROR_NONE)
void*
ves_icall_RuntimeMethodHandle_GetFunctionPointer_raw(MonoMethod* method, MonoError* error);

void* get_fast_callable_managed_function(
        const char_t* dotnet_type,
        const char_t* dotnet_type_method) {

    if (!initialized) {
        load_managed_runtime();
    }

    std::string nmspace;
    std::string type;
    std::string typeAndNamespace;
    std::string assembly;
    std::wstring dtype(dotnet_type);
    parseManagedSignature	(UnicodeToString(dtype), assembly, typeAndNamespace, nmspace, type);

    void* ptr = nullptr;

    /*
     * A pure (naive, no cache, etc) mono implementation would do this.
     * mono_marshal_get_managed_wrapper is not exposed in the microsoft.netcore.app.runtime.mono.[rid] package library,
     * but ves_icall_RuntimeMethodHandle_GetFunctionPointer directly calls it for methods marked with
     * UnmanagedCallersOnly, so it does what we need it to do.

        MonoImage *image = mono_assembly_get_image (entry_assembly);
        MonoClass *klass = mono_class_from_name(image, nmspace.c_str(), type.c_str());
        MonoMethod *method = mono_class_get_method_from_name(klass, dotnet_type_method, -1);

        MonoError error;
        MonoClass *delegate_klass = nullptr;
        MonoGCHandle target_handle = nullptr;
        ptr = ves_icall_RuntimeMethodHandle_GetFunctionPointer_raw(method, &error);
    */

    /*
     * But coreclr_create_delegate was implemented in the Mono VM in .net 7
     * https://github.com/lambdageek/runtime/commit/5c30a82505071936507669f8b6f490b98c1bfb41
     * So we can use the same call on both coreclr and mono.
     * No domain handle or id is needed, Mono doesn't use it
     */

    std::wstring method(dotnet_type_method);
    int rv = coreclr_create_delegate(nullptr, 0, assembly.c_str(), typeAndNamespace.c_str(),
                                     UnicodeToString(method).c_str(),
                                     &ptr);
    return ptr;
}

#if PLATFORM_WIN
#define MONO_API_IMPL(r,f, ...)   \
    r f(__VA_ARGS__)
MONO_API_IMPL(int, coreclr_create_delegate, void *hostHandle, unsigned int domainId,
                    const char *entryPointAssemblyName, const char *entryPointTypeName,
                    const char *entryPointMethodName, void **delegate)
{
    return 0;
}
#endif

MONO_API_IMPL(int, monovm_initialize_preparsed,
                  MonoCoreRuntimeProperties * parsed_properties, int propertyCount, const char **propertyKeys,
                   const char **propertyValues)
{
    return 0;
}
MONO_API_IMPL(void, mono_debug_init, MonoDebugFormat format)
{
    
}

MONO_API_IMPL(MonoAssembly *, mono_assembly_open,
                  const char *filename, MonoImageOpenStatus *status)
{
    return nullptr;
}

MONO_API_IMPL(void, mono_install_assembly_preload_hook,
                  MonoAssemblyPreLoadFunc func, void *user_data)
{
    
}

MONO_API_IMPL(MonoAssemblyName *, mono_assembly_get_name, MonoAssembly * assembly)
{
    return nullptr;
}

MONO_API_IMPL(const char *, mono_assembly_name_get_name, MonoAssemblyName * aname)
{
    return {};
}

MONO_API_IMPL(MonoDomain *, mono_jit_init, const char *root_domain_name)
{
    return nullptr;
}

MONO_API_IMPL(void, mono_set_signal_chaining, mono_bool chain_signals){}

MONO_API_IMPL(void, mono_set_crash_chaining, mono_bool chain_signals){}

MONO_API_IMPL(void, mono_add_internal_call, const char *name, const void *method)
{
    
}

CSH_END_C
#endif
