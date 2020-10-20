#pragma once

#if defined (_MSC_VER)
    #define PURO_MSVC 1
#elif defined (__APPLE_CPP__) || defined (__APPLE_CC__)
    #define PURO_XCODE 1
#elif defined(__GNUC__)
    #define PURO_GCC 1
#endif

#define _C99_MATH 1

#if PURO_MSVC
    #define NOMINMAX
    #include <Windows.h>
    #include <intrin.h> // MSVS breakpoint
    #define breakpoint __debugbreak()
    #define PURO_RESTRICT __restrict
    #define FORCE_INLINE __forceinline
#elif PURO_XCODE
    #define breakpoint { asm ("int $3"); }
    #define PURO_RESTRICT __restrict__
    #define FORCE_INLINE __attribute__((always_inline))
#elif PURO_GCC
    #define breakpoint raise(SIGABRT)
    #define PURO_RESTRICT __restrict__
#endif


#ifndef PURO_DEBUG
    #define PURO_DEBUG 1
#endif

#if PURO_DEBUG == 1
    #define errorif(condition, msg) if ((condition)) { std::cout << msg << "\n"; breakpoint; }
#else
    #define errorif(condition, msg) ((void)0)
#endif


