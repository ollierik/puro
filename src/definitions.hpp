#pragma once

//#include <cassert>
//#include <intrin.h>

#ifndef PURO_DEBUG
    #define PURO_DEBUG 1
#endif

// TODO currently works with Windows only

#if PURO_DEBUG

    #define passertfalse __debugbreak()
    #define passert(condition, msg) if ((condition)) { std::cout << msg << std::endl; passertfalse; }

#else

    #define passertfalse ((void)0)
    #define passert(condition, msg) ((void)0)
#endif



#define PI 3.14159265358979323846 
 
