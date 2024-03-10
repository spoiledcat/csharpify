#ifndef CSHARPIFY_MANAGED_EXPORTS_H_
#define CSHARPIFY_MANAGED_EXPORTS_H_

#include <string>

#undef DNNE_COMPILE_AS_SOURCE
#include "exports.h"

DNNE_EXTERN_C DNNE_API int DNNE_CALLTYPE CallingBackToNativeLand(int number);

#endif // CSHARPIFY_MANAGED_EXPORTS_H_