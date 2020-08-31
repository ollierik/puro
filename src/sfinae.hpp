#pragma once

namespace puro {

template <typename TestType, typename Result> struct enable_if_scalar {};
template <typename Result> struct enable_if_scalar <int, Result>                  { typedef void type; };
template <typename Result> struct enable_if_scalar <unsigned int, Result>         { typedef void type; };
template <typename Result> struct enable_if_scalar <float, Result>                { typedef void type; };
template <typename Result> struct enable_if_scalar <double, Result>               { typedef void type; };
template <typename Result> struct enable_if_scalar <const int, Result>            { typedef void type; };
template <typename Result> struct enable_if_scalar <const unsigned int, Result>   { typedef void type; };
template <typename Result> struct enable_if_scalar <const float, Result>          { typedef void type; };
template <typename Result> struct enable_if_scalar <const double, Result>         { typedef void type; };
    
template <typename TestType, typename Result>
struct enable_if_buffer {};
    
template <int NumChannels, int Length, typename T, typename Result>
struct enable_if_buffer <fixed_buffer<NumChannels, Length, T>, Result> { typedef Result type; };
    
template <int NumChannels, typename T, typename Result>
struct enable_if_buffer <buffer<NumChannels, T>, Result> { typedef Result type; };


} // namespace puro
