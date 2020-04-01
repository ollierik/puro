#pragma once

#if defined (_MSC_VER)
    #define PURO_WINDOWS 1
#endif

#ifndef PURO_DEBUG
    #define PURO_DEBUG 0
#endif

// TODO currently works with Windows only

#if PURO_DEBUG && PURO_WINDOWS
    #include <intrin.h> // MSVS breakpoint
    #define passertfalse __debugbreak()
    #define passert(condition, msg) if ((condition)) { std::cout << msg << std::endl; passertfalse; }

#else

    #define passertfalse ((void)0)
    #define passert(condition, msg) ((void)0)
#endif



#define PI 3.14159265358979323846 
 
