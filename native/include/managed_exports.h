#ifndef CSHARPIFY_MANAGED_EXPORTS_H_
#define CSHARPIFY_MANAGED_EXPORTS_H_

#include <string>

#undef DNNE_COMPILE_AS_SOURCE
#include "exports.h"

#ifdef DNNE_WINDOWS
    #ifdef _WCHAR_T_DEFINED
        typedef wchar_t char_t;
        typedef std::wstring string_t;
    #else
        typedef unsigned short char_t;
        typedef std::string string_t;
    #endif
#else
    typedef char char_t;
    typedef std::string string_t;
#endif

#endif // CSHARPIFY_MANAGED_EXPORTS_H_