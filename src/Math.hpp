#pragma once

/** Maths routines for buffers. Used to allow flexibility later on by implementing vector math libs such as IPP */
struct Math
{
    template <typename FloatType>
    static FloatType pi()
    {
        return static_cast<FloatType> (3.14159265358979323846);
    }

    template <typename FloatType>
    static void multiplyAdd(FloatType* dst, const FloatType* src1, const FloatType* src2, const int n)
    {
        for (int i = 0; i < n; ++i)
            dst[i] += src1[i] * src2[i];
    };

    /** Multiply src buffer with value and set to dst */
    template <typename FloatType>
    static void multiplySet(FloatType* dst, const FloatType* src, const FloatType value, const int n)
    {
        for (int i = 0; i < n; ++i)
            dst[i] += src[i] * value;
    }

    /** In-place sin */
    template <typename FloatType>
    static void sin(FloatType* buf, const int n)
    {
        for (int i=0; i<n; ++i)
            buf[i] = std::sin(buf[i]);
    }

    /** In-place cosine */
    template <typename FloatType>
    static void cos(FloatType* buf, const int n)
    {
        for (int i=0; i<n; ++i)
            buf[i] = std::cos(buf[i]);
    }

    /** Copy from source to destination */
    template <typename FloatType>
    static void copy(FloatType* dst, FloatType* src, const int n)
    {
        for (int i=0; i<n; ++i)
            dst[i] = src[i];
    }

    /** Set to constant */
    template <typename FloatType>
    static void add(FloatType* buf, const int n, FloatType value)
    {
        for (int i=0; i<n; ++i)
            buf[i] += value;
    }

    /** Set to constant */
    template <typename FloatType>
    static void set(FloatType* buf, const int n, FloatType value)
    {
        for (int i=0; i<n; ++i)
            buf[i] = value;
    }

};
