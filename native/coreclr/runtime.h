#ifndef CSHARPIFY_CORECLR_RUNTIME_H_
#define CSHARPIFY_CORECLR_RUNTIME_H_

#ifdef _WIN32
#define ssize_t long long
#endif

#include <cstddef>
#include <cstdint>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* GCHandle;

/* This is copied from eglib's header files */
typedef unsigned int   guint;
typedef int32_t        gboolean;
typedef int32_t        gint32;
typedef uint32_t       guint32;
typedef char           gchar;
typedef void *         gpointer;
typedef const void *   gconstpointer;
typedef guint    (*GHashFunc)      (gconstpointer key);
typedef gboolean (*GEqualFunc)     (gconstpointer a, gconstpointer b);

#ifndef GPOINTER_TO_UINT
#define GPOINTER_TO_UINT(p) ((uint32_t)  (long) (p))
#endif

#ifndef GINT_TO_POINTER
#define GINT_TO_POINTER(i) ((void *) (long) (i))
#endif


/* mini/mono-private-unstable.h */

typedef struct {
	uint32_t assembly_count;
	char **basenames; /* Foo.dll */
	uint32_t *basename_lens;
	char **assembly_filepaths; /* /blah/blah/blah/Foo.dll */
} MonoCoreTrustedPlatformAssemblies;

typedef struct {
	uint32_t dir_count;
	char **dirs;
} MonoCoreLookupPaths;

typedef struct {
	MonoCoreTrustedPlatformAssemblies *trusted_platform_assemblies;
	MonoCoreLookupPaths *app_paths;
	MonoCoreLookupPaths *native_dll_search_directories;
	void* pinvoke_override;
} MonoCoreRuntimeProperties;


// static MonoCoreRuntimeProperties monovm_core_properties;

/* This is copied from mono's header files */

/* utils/mono-publib.h */
#define MONO_API
typedef int32_t	mono_bool;

/* metadata/image.h */
#if defined (CORECLR_RUNTIME)
// In Mono, MonoAssembly is not related to MonoObject, but for the CoreCLR bridge we use the same memory representation for both types.
typedef struct _MonoObject MonoAssembly;
#else
typedef struct _MonoAssembly MonoAssembly;
#endif
typedef struct _MonoAssemblyName MonoAssemblyName;
typedef struct _MonoImage MonoImage;

typedef enum {
	MONO_IMAGE_OK,
	MONO_IMAGE_ERROR_ERRNO,
	MONO_IMAGE_MISSING_ASSEMBLYREF,
	MONO_IMAGE_IMAGE_INVALID
} MonoImageOpenStatus;

/* metadata/metadata.h */
#if defined (CORECLR_RUNTIME)
// In Mono, MonoClass is not related to MonoObject at all, but for the CoreCLR bridge we use the same struct representation for both types.
typedef struct _MonoObject MonoClass;
#else
typedef struct _MonoClass MonoClass;
#endif
typedef struct _MonoDomain MonoDomain;
#if defined (CORECLR_RUNTIME)
// In Mono, MonoMethod is not related to MonoObject at all, but for the CoreCLR bridge we use the same struct representation for both types.
typedef struct _MonoObject MonoMethod;
#else
typedef struct _MonoMethod MonoMethod;
#endif
typedef struct _MonoMethodSignature MonoMethodSignature;
#if defined (CORECLR_RUNTIME)
// In Mono, MonoType is not related to MonoObject at all, but for the CoreCLR bridge we use the same struct representation for both types.
typedef struct _MonoObject MonoType;
#else
typedef struct _MonoType MonoType;
#endif

/* metadata/class.h */
typedef struct MonoVTable MonoVTable;

typedef struct _MonoClassField MonoClassField;

/* metadata/object.h */
#if defined (CORECLR_RUNTIME)
// In Mono, these types are substructs of MonoObject, but for the CoreCLR bridge we use the same struct representation for both types.
typedef struct _MonoObject MonoString;
typedef struct _MonoObject MonoArray;
typedef struct _MonoObject MonoReflectionAssembly;
typedef struct _MonoObject MonoReflectionMethod;
typedef struct _MonoObject MonoReflectionType;
typedef struct _MonoObject MonoException;
#else
typedef struct _MonoString MonoString;
typedef struct _MonoArray MonoArray;
typedef struct _MonoReflectionAssembly MonoReflectionAssembly;
typedef struct _MonoReflectionMethod MonoReflectionMethod;
typedef struct _MonoReflectionType MonoReflectionType;
typedef struct _MonoException MonoException;
#endif
typedef struct _MonoThread MonoThread;
typedef struct _MonoThreadsSync MonoThreadsSync;
typedef struct _MonoObject MonoObject;

#if defined (CORECLR_RUNTIME)
// In Mono, MonoReferenceQueue is not related to MonoObject at all, but for the CoreCLR bridge we use the same struct representation for both types.
typedef struct _MonoObject MonoReferenceQueue;
#else
typedef struct _MonoReferenceQueue MonoReferenceQueue;
#endif
typedef void (*mono_reference_queue_callback) (void *user_data);

#if !defined (CORECLR_RUNTIME)
#define mono_array_addr(array,type,index) ((type*)(void*) mono_array_addr_with_size (array, sizeof (type), index))
#define mono_array_get(array,type,index) ( *(type*)mono_array_addr ((array), type, (index)) )
#define mono_array_setref(array,index,value,exception_gchandle)	\
	do {	\
		void **__p = (void **) mono_array_addr ((array), void*, (index));	\
		mono_gc_wbarrier_set_arrayref ((array), __p, (MonoObject*)(value));	\
		/* *__p = (value);*/	\
	} while (0)
#endif // !defined (CORECLR_RUNTIME)

/* metadata/assembly.h */

typedef MonoAssembly * (*MonoAssemblyPreLoadFunc) (MonoAssemblyName *aname, char **assemblies_path, void* user_data);

/* metadata/profiler.h */
typedef struct _MonoProfiler MonoProfiler;

typedef enum {
	MONO_PROFILE_NONE = 0,
	MONO_PROFILE_APPDOMAIN_EVENTS = 1 << 0,
	MONO_PROFILE_ASSEMBLY_EVENTS  = 1 << 1,
	MONO_PROFILE_MODULE_EVENTS    = 1 << 2,
	MONO_PROFILE_CLASS_EVENTS     = 1 << 3,
	MONO_PROFILE_JIT_COMPILATION  = 1 << 4,
	MONO_PROFILE_INLINING         = 1 << 5,
	MONO_PROFILE_EXCEPTIONS       = 1 << 6,
	MONO_PROFILE_ALLOCATIONS      = 1 << 7,
	MONO_PROFILE_GC               = 1 << 8,
	MONO_PROFILE_THREADS          = 1 << 9,
	MONO_PROFILE_REMOTING         = 1 << 10,
	MONO_PROFILE_TRANSITIONS      = 1 << 11,
	MONO_PROFILE_ENTER_LEAVE      = 1 << 12,
	MONO_PROFILE_COVERAGE         = 1 << 13,
	MONO_PROFILE_INS_COVERAGE     = 1 << 14,
	MONO_PROFILE_STATISTICAL      = 1 << 15,
	MONO_PROFILE_METHOD_EVENTS    = 1 << 16,
	MONO_PROFILE_MONITOR_EVENTS   = 1 << 17,
	MONO_PROFILE_IOMAP_EVENTS     = 1 << 18, /* this should likely be removed, too */
	MONO_PROFILE_GC_MOVES         = 1 << 19,
	MONO_PROFILE_GC_ROOTS         = 1 << 20
} MonoProfileFlags;

typedef enum {
	MONO_GC_EVENT_START,
	MONO_GC_EVENT_MARK_START,
	MONO_GC_EVENT_MARK_END,
	MONO_GC_EVENT_RECLAIM_START,
	MONO_GC_EVENT_RECLAIM_END,
	MONO_GC_EVENT_END,
	MONO_GC_EVENT_PRE_STOP_WORLD,
	MONO_GC_EVENT_POST_STOP_WORLD,
	MONO_GC_EVENT_PRE_START_WORLD,
	MONO_GC_EVENT_POST_START_WORLD
} MonoGCEvent;

typedef void (*MonoProfileFunc) (MonoProfiler *prof);
typedef void (*MonoProfileThreadFunc) (MonoProfiler *prof, uintptr_t tid);
typedef void (*MonoProfileGCFunc)         (MonoProfiler *prof, MonoGCEvent event, int generation);
typedef void (*MonoProfileGCResizeFunc)   (MonoProfiler *prof, int64_t new_size);

/* metadata/mono-debug.h */

typedef enum {
	MONO_DEBUG_FORMAT_NONE,
	MONO_DEBUG_FORMAT_MONO,
	/* Deprecated, the mdb debugger is not longer supported. */
	MONO_DEBUG_FORMAT_DEBUGGER
} MonoDebugFormat;

/* mini/debugger-agent.h */

typedef struct {
	const char *name;
	void (*connect) (const char *address);
	void (*close1) (void);
	void (*close2) (void);
	gboolean (*send) (void *buf, size_t len);
	ssize_t (*recv) (void *buf, size_t len);
} DebuggerTransport;

/* metadata/mini.h */
typedef gboolean (*MonoExceptionFrameWalk)      (MonoMethod *method, gpointer ip, size_t native_offset, gboolean managed, gpointer user_data);
typedef void  (*MonoUnhandledExceptionFunc)         (MonoObject *exc, gpointer user_data);

typedef unsigned char* (*MonoLoadAotDataFunc)          (MonoAssembly *assembly, int size, gpointer user_data, void **out_handle);
typedef void  (*MonoFreeAotDataFunc)          (MonoAssembly *assembly, int size, gpointer user_data, void *handle);

/* metadata/blob.h */

/*
 * Encoding for type signatures used in the Metadata
 */
typedef enum {
	MONO_TYPE_END        = 0x00,       /* End of List */
	MONO_TYPE_VOID       = 0x01,
	MONO_TYPE_BOOLEAN    = 0x02,
	MONO_TYPE_CHAR       = 0x03,
	MONO_TYPE_I1         = 0x04,
	MONO_TYPE_U1         = 0x05,
	MONO_TYPE_I2         = 0x06,
	MONO_TYPE_U2         = 0x07,
	MONO_TYPE_I4         = 0x08,
	MONO_TYPE_U4         = 0x09,
	MONO_TYPE_I8         = 0x0a,
	MONO_TYPE_U8         = 0x0b,
	MONO_TYPE_R4         = 0x0c,
	MONO_TYPE_R8         = 0x0d,
	MONO_TYPE_STRING     = 0x0e,
	MONO_TYPE_PTR        = 0x0f,       /* arg: <type> token */
	MONO_TYPE_BYREF      = 0x10,       /* arg: <type> token */
	MONO_TYPE_VALUETYPE  = 0x11,       /* arg: <type> token */
	MONO_TYPE_CLASS      = 0x12,       /* arg: <type> token */
	MONO_TYPE_VAR	     = 0x13,	   /* number */
	MONO_TYPE_ARRAY      = 0x14,       /* type, rank, boundsCount, bound1, loCount, lo1 */
	MONO_TYPE_GENERICINST= 0x15,	   /* <type> <type-arg-count> <type-1> \x{2026} <type-n> */
	MONO_TYPE_TYPEDBYREF = 0x16,
	MONO_TYPE_I          = 0x18,
	MONO_TYPE_U          = 0x19,
	MONO_TYPE_FNPTR      = 0x1b,	      /* arg: full method signature */
	MONO_TYPE_OBJECT     = 0x1c,
	MONO_TYPE_SZARRAY    = 0x1d,       /* 0-based one-dim-array */
	MONO_TYPE_MVAR	     = 0x1e,       /* number */
	MONO_TYPE_CMOD_REQD  = 0x1f,       /* arg: typedef or typeref token */
	MONO_TYPE_CMOD_OPT   = 0x20,       /* optional arg: typedef or typref token */
	MONO_TYPE_INTERNAL   = 0x21,       /* CLR internal type */

	MONO_TYPE_MODIFIER   = 0x40,       /* Or with the following types */
	MONO_TYPE_SENTINEL   = 0x41,       /* Sentinel for varargs method signature */
	MONO_TYPE_PINNED     = 0x45,       /* Local var that points to pinned object */

	MONO_TYPE_ENUM       = 0x55        /* an enumeration */
} MonoTypeEnum;

/*
 * From internal headers
 */

/* metadata/gc-internal.h */

enum {
   MONO_GC_FINALIZER_EXTENSION_VERSION = 1,
};

typedef struct {
	int version;
	gboolean (*is_class_finalization_aware) (MonoClass *klass);
	void (*object_queued_for_finalization) (MonoObject *object);
} MonoGCFinalizerCallbacks;

/* metadata/sgen-toggleref.h */

typedef enum {
	MONO_TOGGLE_REF_DROP,
	MONO_TOGGLE_REF_STRONG,
	MONO_TOGGLE_REF_WEAK
} MonoToggleRefStatus;

typedef MonoToggleRefStatus (*MonoToggleRefCallback) (MonoObject *obj);

/* metadata/mono-hash.h */

typedef enum {
	MONO_HASH_KEY_GC = 1,
	MONO_HASH_VALUE_GC = 2,
	MONO_HASH_KEY_VALUE_GC = MONO_HASH_KEY_GC | MONO_HASH_VALUE_GC,
} MonoGHashGCType;

#if defined (CORECLR_RUNTIME)
// In Mono, MonoGHashTable is not related to MonoObject, but for the CoreCLR bridge we use the same memory representation for both types.
typedef struct _MonoObject MonoGHashTable;
#else
typedef struct _MonoGHashTable MonoGHashTable;
#endif

/* utils/mono-logger.h */

typedef void (*MonoLogCallback) (const char *log_domain, const char *log_level, const char *message, mono_bool fatal, void *user_data);
typedef void (*MonoPrintCallback) (const char *string, mono_bool is_stdout);

/* mini/jit.h */
typedef enum {
	MONO_AOT_MODE_NONE,
	MONO_AOT_MODE_NORMAL,
	MONO_AOT_MODE_HYBRID,
	MONO_AOT_MODE_FULL,
	MONO_AOT_MODE_LLVMONLY,
	MONO_AOT_MODE_INTERP,
	MONO_AOT_MODE_INTERP_LLVMONLY,
	MONO_AOT_MODE_LLVMONLY_INTERP,
	MONO_AOT_MODE_INTERP_ONLY,
} MonoAotMode;

/* metadata/marshal.h */

// The 'gchandle' parameter is defined as a 'guint32' in mono/mono's 2020-02 branch, and as a 'MonoGCHandle' (aka void*) in mono/mono's main branch.
// Here we go with the latter (void*), because that's the future, and it should also be compatible with the 2020-02 branch.
typedef void (*MonoFtnPtrEHCallback) (GCHandle gchandle);

/* mini/mono-private-unstable.h */
typedef struct {
	uint32_t kind; // 0 = Path of runtimeconfig.blob, 1 = pointer to image data, >= 2 undefined
	union {
		struct {
			const char *path;
		} name;
		struct {
			const char *data;
			uint32_t data_len;
		} data;
	} runtimeconfig;
} MonovmRuntimeConfigArguments;

typedef void (*MonovmRuntimeConfigArgumentsCleanup)          (MonovmRuntimeConfigArguments *args, void *user_data);

/* not in any header */

void mono_gc_init_finalizer_thread ();

#define MONO_API_FUNCTION(ret,name,args) MONO_API ret name args;
MONO_API_FUNCTION(int, monovm_initialize_preparsed, (MonoCoreRuntimeProperties *parsed_properties, int propertyCount, const char **propertyKeys, const char **propertyValues))
#undef MONO_API_FUNCTION


#ifdef __cplusplus
}
#endif

#endif // CSHARPIFY_CORECLR_RUNTIME_H_