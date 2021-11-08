#pragma once

#ifndef CPP_NORMALISE_MACRO_GUARD
#define CPP_NORMALISE_MACRO_GUARD


/********************************************
 ** Windows MSVC
 *******************************************/
#if defined (_MSC_VER)

#define NOMINMAX // get rid of min and max macros on MSVC

#ifndef RESTRICT
    #define RESTRICT __restrict
#endif

#ifndef FORCE_INLINE
    #define FORCE_INLINE __forceinline
#define FORCE_INLINE

/********************************************
 **** Windows MSVC - Debug
 *******************************************/
#if defined(DEBUG) || defined(_DEBUG)

#include <Windows.h>    // OutputDebugStringA
#include <intrin.h>     // __debugbreak() 
#include <chrono>       // debug_print time functions

#ifndef breakpoint
    #define breakpoint __debugbreak()
#endif

#ifndef debug_print
    #define debug_print(msg, ...)                                                               \
    do {                                                                                        \
        char buf[256];                                                                          \
        const auto epoch = std::chrono::system_clock::now().time_since_epoch();                 \
        const auto timeunits = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);    \
        const long unsigned int timeu = timeunits.count();                                      \
        sprintf_s(buf, "%lu: ", timeu);                                                         \
        OutputDebugStringA(buf);                                                                \
        sprintf_s(buf, msg, __VA_ARGS__);                                                       \
        OutputDebugStringA(buf);                                                                \
        OutputDebugStringA("\n");                                                               \
    } while(0);
#endif

#ifndef errorif
    #define errorif(condition, msg, ...)                                                        \
    do {                                                                                        \
        if ((condition)) {                                                                      \
            debug_print(msg "\n", __VA_ARGS__); breakpoint;                                     \
        }                                                                                       \
    } while (0);
#endif

#endif // if DEBUG

#endif // if Windows

/********************************************
** Apple Xcode
********************************************/
#if defined(__APPLE_CPP__)

#ifndef RESTRICT
    #define RESTRICT __restrict__
#endif

#ifndef FORCE_INLINE
    #define FORCE_INLINE __attribute__((always_inline))
#endif

/********************************************
 ***** Apple Xcode - Debug
********************************************/
#if defined(DEBUG) || defined(_DEBUG)

#ifndef(breakpoint)
    #define breakpoint __builtin_trap();
#endif

#ifndef debug_print
    #define debug_print(msg, ...)                                                               \
    do {                                                                                        \
        char buf[256];                                                                          \
        const auto epoch = std::chrono::system_clock::now().time_since_epoch();                 \
        const auto timeunits = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);    \
        const long unsigned int timeu = timeunits.count();                                      \
        sprintf(buf, "%lu: ", timeu);                                                           \
        printf("%s", buf);                                                                      \
        sprintf(buf, msg, __VA_ARGS__);                                                         \
        printf("%s\n", buf);                                                                    \
    } while(0);
#endif

#ifndef errorif
    #define errorif(condition, msg, ...)                                                        \
    do {                                                                                        \
        if ((condition)) {                                                                      \
            debug_print(msg "\n", __VA_ARGS__); breakpoint;                                     \
        }                                                                                       \
    } while (0);
#endif

#endif // if DEBUG

#endif // if Apple

#if defined(__GNUC__)
    // TODO
#endif

/********************************************
 ** void defintions as fallback
 *******************************************/

#ifndef RESTRICT
    #define RESTRICT
#endif

#ifndef FORCE_INLINE
    #define FORCE_INLINE
#endif

#ifndef breakpoint
    #define breakpoint ((void)0);
#endif

#ifndef debug_print
    #define debug_print(msg, ...) ((void)0)
#endif

#ifndef errorif
    #define errorif(condition, msg, ...) ((void)0)
#endif


#endif // CPP_BOOTSTRAP



