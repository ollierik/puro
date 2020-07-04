#pragma once

/** Maths routines, mostly for buffers. Used to allow flexibility later on by implementing vector math libs such as IPP */
namespace math
{
    template <typename FloatType>
    FloatType pi()
    {
        return static_cast<FloatType> (3.14159265358979323846);
    }

    template <typename ValueType>
    ValueType min(ValueType f1, ValueType f2)
    {
        return f1 < f2 ? f1 : f2;
    }

    template <typename ValueType>
    ValueType max(ValueType f1, ValueType f2)
    {
        return f1 > f2 ? f1 : f2;
    }

    template <typename ValueType>
    ValueType clip(ValueType val, ValueType minValue, ValueType maxValue)
    {
        return min(max(val, minValue), maxValue);
    }

    template <typename FloatType, typename ValueType=int>
    ValueType round(FloatType value)
    {
        return static_cast<ValueType> (value + (FloatType)0.5);
    }

    template <typename FloatType>
    void multiply_inplace(FloatType* dst, const FloatType* src, const int n)
    {
        for (int i = 0; i < n; ++i)
            dst[i] *= src[i];
    };

    template <typename FloatType>
    void multiply_add(FloatType* dst, const FloatType* src1, const FloatType* src2, const int n)
    {
        for (int i = 0; i < n; ++i)
            dst[i] += src1[i] * src2[i];
    };

    template <typename FloatType>
    void multiply_add(FloatType* dst, const FloatType* src, const FloatType value, const int n)
    {
        for (int i = 0; i < n; ++i)
            dst[i] += src[i] * value;
    };

    /** Multiply src buffer with value and set to dst */
    template <typename FloatType>
    void multiply_set(FloatType* dst, const FloatType* src, const FloatType value, const int n)
    {
        for (int i = 0; i < n; ++i)
            dst[i] = src[i] * value;
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

    /** Add constant */
    template <typename FloatType>
    void add(FloatType* buf, FloatType value, const int n)
    {
        for (int i=0; i<n; ++i)
            buf[i] += value;
    }

    /** Set to constant */
    template <typename FloatType>
    void set(FloatType* buf, FloatType value, const int n)
    {
        for (int i=0; i<n; ++i)
            buf[i] = value;
    }

};
