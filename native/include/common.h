#ifndef CSHARPIFY_COMMON_H
#define CSHARPIFY_COMMON_H

#if defined(_MSC_VER) && !defined(__clang__) && !defined(__INTEL_COMPILER)
#   define COMPILER_MSVC 1
#elif defined(__clang)
#   define COMPILER_CLANG 1
#elif defined(__GNUC__)
#   define COMPILER_GNUC 1
#endif

#if !defined(PLATFORM_CONFIGURED)

//#define PLATFORM_WIN 0
//#define PLATFORM_LINUX 0
//#define PLATFORM_MAC 0
//#define PLATFORM_IOS 0
//#define PLATFORM_TVOS 0
//#define PLATFORM_WATCHOS 0
//#define PLATFORM_VISIONOS 0
//#define PLATFORM_ANDROID 0
//#define PLATFORM_ARCH_32 0
//#define PLATFORM_ARCH_64 0

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__CYGWIN32__)
#   define PLATFORM_WIN 1
#elif defined(__MACH__) || defined(__APPLE__)

#   include <TargetConditionals.h>

#if TARGET_OS_OSX == 1
#   define PLATFORM_MAC 1
#else
#if TARGET_OS_SIMULATOR == 1
#   define PLATFORM_IOS_SIMULATOR 1
#endif

#if TARGET_OS_MACCATALYST == 1
#   define PLATFORM_MACCATALYST 1
#endif

#if TARGET_OS_IOS == 1
#   define PLATFORM_IOS 1
#elif TARGET_OS_TV == 1
#   define PLATFORM_TVOS 1
#elif TARGET_OS_WATCH == 1
#   define PLATFORM_WATCHOS 1
#elif TARGET_OS_VISION == 1
#   define PLATFORM_VISIONOS 1
#endif
#endif

#elif defined(__ANDROID__)
#   define PLATFORM_ANDROID 1
#elif defined(__linux__)
#   define PLATFORM_LINUX 1
#endif

#if defined(_AMD64_) || defined(__LP64__) || defined(_M_ARM64)
#   define PLATFORM_ARCH_64 1
#else
#   define PLATFORM_ARCH_32 1
#endif

#define PLATFORM_CONFIGURED 1
#endif

#if PLATFORM_WIN
#   ifdef _WCHAR_T_DEFINED
        typedef wchar_t char_t;
#   else
        typedef unsigned short char_t;
#   endif
#else
    typedef char char_t;
#endif

#if PLATFORM_WIN
#   define CSH_EXPORT_API __declspec(dllexport)
#   define CSH_CALLTYPE __stdcall
#   define CSH_STR_(s1) L ## s1
#   define CSH_STR(s) CSH_STR_(s)
#else
#   define CSH_EXPORT_API __attribute__((visibility("default")))
#   define CSH_CALLTYPE __stdcall
#   define CSH_STR(s) s
#endif

#define CSH_BEGIN_C  extern "C" {
#define CSH_END_C    }
#define CSH_EXTERN_C extern "C"

#define CSH_TOSTRING2(s) #s
#define CSH_TOSTRING(s) CSH_TOSTRING2(s)

#endif //CSHARPIFY_COMMON_H
