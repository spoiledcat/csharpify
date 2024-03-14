#include "config.h"
#include "common.h"

#if RUNTIME_MONO

#include "managed_exports.h"
#include "bridge.h"
#include "utils.h"

#include <mono/metadata/mono-debug.h>
#include <mono/metadata/assembly.h>
#include <mono/jit/jit.h>
#include <mono/utils/mono-error.h>

#include <filesystem>
#include <cassert>
#include <dlfcn.h>

namespace fs = std::filesystem;

static bool initialized;

static MonoAssembly* entry_assembly = nullptr;

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
        return mono_assembly_open(possible.c_str(), nullptr);
    }

    possible = fs::path(basePath / name);
    if (exists(possible)) {
        return mono_assembly_open(possible.c_str(), nullptr);
    }

    return nullptr;
}

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

CSHARPIFY_BEGIN_C
void* pinvoke_override(const char* libraryName, const char* entrypointName) {
    void* symbol = nullptr;
    if (!strcmp(libraryName, "__Internal") ||
        !strcmp(libraryName, "cimgui")) {
        symbol = load_symbol(nullptr, entrypointName);
    }
    return symbol;
}
CSHARPIFY_END_C


char* strdup_printf(int* len, const char* msg, ...) {
    // COOP: no managed memory access: any mode
    va_list args;
    char* formatted = NULL;

    va_start (args, msg);
    *len = vasprintf(&formatted, msg, args);
    va_end (args);

    return formatted;
}

int load_managed_runtime() {
    fs::path assemblyPath = ".";
    auto assemblyName = ASSEMBLYNAME ".dll";
    fs::path startAssembly = assemblyPath / assemblyName;

    auto basePath = normalizePath(".");
    auto runtimePath = fs::path{normalizePath("sdk")};

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
}

#ifdef __cplusplus
extern "C" {
#endif

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
    parseManagedSignature(dotnet_type, assembly, typeAndNamespace, nmspace, type);

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

    int rv = coreclr_create_delegate(nullptr, 0, assembly.c_str(), typeAndNamespace.c_str(), dotnet_type_method, &ptr);
    return ptr;
}

#ifdef __cplusplus
}
#endif

#endif
