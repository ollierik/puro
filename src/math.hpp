#pragma once

/** Maths routines, mostly for buffers. Used to allow flexibility later on by implementing vector math libs such as IPP */
namespace math
{
    template <typename FloatType>
    FloatType pi()
    {
        return static_cast<FloatType> (3.14159265358979323846);
    }

    template <typename FloatType>
    void multiply_add(FloatType* dst, const FloatType* src1, const FloatType* src2, const int n)
    {
        for (int i = 0; i < n; ++i)
            dst[i] += src1[i] * src2[i];
    };

    /** Multiply src buffer with value and set to dst */
    template <typename FloatType>
    void multiply_set(FloatType* dst, const FloatType* src, const FloatType value, const int n)
    {
        for (int i = 0; i < n; ++i)
            dst[i] += src[i] * value;
    }

    /** In-place sin */
    template <typename FloatType>
    void sin(FloatType* buf, const int n)
    {
        for (int i=0; i<n; ++i)
            buf[i] = std::sin(buf[i]);
    }

    /** In-place cosine */
    template <typename FloatType>
    void cos(FloatType* buf, const int n)
    {
        for (int i=0; i<n; ++i)
            buf[i] = std::cos(buf[i]);
    }

    /** Copy from source to destination */
    template <typename FloatType>
    void copy(FloatType* dst, FloatType* src, const int n)
    {
        for (int i=0; i<n; ++i)
            dst[i] = src[i];
    }

    /** Add from source to destination */
    template <typename FloatType>
    void add(FloatType* dst, FloatType* src, const int n)
    {
        for (int i=0; i<n; ++i)
            dst[i] += src[i];
    }

    /** Set to constant */
    template <typename FloatType>
    void add(FloatType* buf, const int n, FloatType value)
    {
        for (int i=0; i<n; ++i)
            buf[i] += value;
    }

    /** Set to constant */
    template <typename FloatType>
    void set(FloatType* buf, const int n, FloatType value)
    {
        for (int i=0; i<n; ++i)
            buf[i] = value;
    }

};
