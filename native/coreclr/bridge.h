#if defined (CORECLR_RUNTIME)

#ifndef CSHARPIFY_CORECLR_BRIDGE_H_
#define CSHARPIFY_CORECLR_BRIDGE_H_

#include "runtime.h"
#include "coreclrhost.h"

#define LOG_CORECLR(...)
//#define LOG_CORECLR(...) fprintf (__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

// Representation of a managed object in native code. In Mono,the MonoObjects
// are tracked by the GC (which scans the stack), but we can't make CoreCLR
// scan the stack, so we use a reference counted version of MonoObject instead -
// - we just put the GCHandle into a reference counted MonoObject, and when the
// MonoObject is freed, then we free the GCHandle as well.
//
// This struct must be kept in sync with the MonoObject struct in Runtime.CoreCLR.cs
struct _MonoObject {
	uint32_t reference_count;
	GCHandle gchandle;
	// We write the value of the struct here every time we create a MonoObject instance.
	// We can also fetch this value when it's needed (as opposed to creating it every time),
	// but that runs into threading issues (what if two threads needs it at the same time?).
	// Nothing unsolvable, but I'm going with the simplest solution until this is proven
	// to be a problem.
	void *struct_value;
};

// This struct must be kept in sync with the MonoMethodSignature struct in Runtime.CoreCLR.cs
struct _MonoMethodSignature {
	MonoObject *method;
	int parameter_count;
	MonoObject *return_type;
	MonoObject *parameters[];
};



#ifdef __cplusplus
}
#endif

#endif // CSHARPIFY_CORECLR_BRIDGE_H_
#endif // CORECLR_RUNTIME