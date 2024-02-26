#ifndef __CSHARPIFY_MANAGED_EXPORTS_H__
#define __CSHARPIFY_MANAGED_EXPORTS_H__

#undef DNNE_COMPILE_AS_SOURCE
#include "exports.h"

#ifdef DNNE_WINDOWS
    #ifdef _WCHAR_T_DEFINED
        typedef wchar_t char_t;
    #else
        typedef unsigned short char_t;
    #endif
#else
    typedef char char_t;
#endif

#endif