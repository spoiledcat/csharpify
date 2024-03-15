#ifndef CSHARPIFY_MANAGED_EXPORTS_H_
#define CSHARPIFY_MANAGED_EXPORTS_H_

#include <string>

#undef DNNE_COMPILE_AS_SOURCE

#include "exports.h"

DNNE_EXTERN_C DNNE_API int DNNE_CALLTYPE CallingBackToNativeLand(int number);
DNNE_EXTERN_C DNNE_API void DNNE_CALLTYPE send_utf16(uint16_t* str);
DNNE_EXTERN_C DNNE_API void DNNE_CALLTYPE send_utf8(char* str);

#endif // CSHARPIFY_MANAGED_EXPORTS_H_