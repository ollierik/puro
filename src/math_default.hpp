#pragma once

#include "../include/pffft.h"

/** Maths routines, mostly for buffers. Used to allow flexibility later on by implementing vector math libs such as IPP */
namespace puro {
namespace math {

/** FFT wrapper for pffft. Results are not normalised!
    irfft(rfft(signal)) = fftSize * signal */
struct fft
{
    fft(int size) : setup(pffft_new_setup(size, PFFFT_REAL)), n(size)
    {}

    ~fft()
    {
        pffft_destroy_setup(setup);
    }

    void rfft(float* dst, float* src)
    {
        pffft_transform_ordered(setup, src, dst, 0, PFFFT_FORWARD);
    }

    void irfft(float* dst, float* src, bool normalise=true)
    {
        pffft_transform_ordered(setup, src, dst, 0, PFFFT_BACKWARD);
    }

    int length() const
    {
        return n;
    }

    PFFFT_Setup* setup;
    int n;
};

/** Memory-aligned allocator for pffft */
template <typename T=float>
struct allocator
{
    typedef T value_type;

    T* allocate(std::size_t n, const void* hint=0)
    {
        auto numBytes = sizeof(T) * n;
        void* mem = pffft_aligned_malloc(numBytes);
        return reinterpret_cast<T*> (mem);
    }

    void deallocate(T* ptr, std::size_t)
    {
        pffft_aligned_free(ptr);
    }
};

constexpr auto pi = 3.14159265358979323846;

    /*
template <typename FloatType>
FloatType pi() noexcept
{
    return static_cast<FloatType> (3.14159265358979323846);
}
     */

template <typename ValueType>
ValueType min(ValueType f1, ValueType f2) noexcept
{
    return f1 < f2 ? f1 : f2;
}

template <typename ValueType>
ValueType max(ValueType f1, ValueType f2) noexcept
{
    return f1 > f2 ? f1 : f2;
}

template <typename ValueType>
ValueType clip(ValueType val, ValueType minValue, ValueType maxValue) noexcept
{
    return min(max(val, minValue), maxValue);
}

template <typename ValueType>
ValueType wrap(ValueType index, ValueType length) noexcept
{
    return index < length ? index : index - length;
}

template <typename ValueType>
ValueType normalise(ValueType value, ValueType zero, ValueType one) noexcept
{
    return (value - zero) / (one - zero);
}

template <typename ValueType>
ValueType scale(ValueType value, ValueType min, ValueType max) noexcept
{
    return value * (max - min) + min;
}

template <typename ValueType>
ValueType atodb(ValueType value, ValueType nonzero=1e-30) noexcept
{
    return 20 * log10(value);
}

template <typename ValueType>
ValueType dbtoa(ValueType value) noexcept
{
    return pow(static_cast<ValueType>(10), value / static_cast<ValueType>(20));
}

/** Frequency to log2 scale */
template <typename ValueType>
ValueType ftox_log2(ValueType freq, const ValueType minLog2= -10.784634845557521, const ValueType maxLog2=-1)
{
    return normalise(log2(freq), minLog2, maxLog2);
}

/** Log2 scale normalised x to frequency */
template <typename ValueType>
ValueType xtof_log2(ValueType value, const ValueType minLog2= -10.784634845557521, const ValueType maxLog2=-1)
{
    return pow(static_cast<ValueType> (2), scale(value, minLog2, maxLog2));
}

template <typename FloatType, typename ValueType=int>
ValueType round(FloatType value) noexcept
{
    return static_cast<ValueType> (value + (FloatType)0.5);
}

/** Compare equality with error */
template <typename FloatType>
bool equal(FloatType f1, FloatType f2, const FloatType epsilon=1e-5)
{
    return (abs(f1-f2) < epsilon);
}

template <typename FloatType>
void multiply(FloatType* dst, const FloatType value, const int n) noexcept
{
    for (int i = 0; i < n; ++i)
        dst[i] *= value;
};

template <typename FloatType>
void multiply(FloatType* dst, const FloatType* src, const int n) noexcept
{
    for (int i = 0; i < n; ++i)
        dst[i] *= src[i];
};

template <typename FloatType>
void multiply_add(FloatType* dst, const FloatType* src1, const FloatType* src2, const int n) noexcept
{
    for (int i = 0; i < n; ++i)
        dst[i] += src1[i] * src2[i];
};

template <typename FloatType>
void multiply_add(FloatType* dst, const FloatType* src, const FloatType value, const int n) noexcept
{
    for (int i = 0; i < n; ++i)
        dst[i] += src[i] * value;
};

/** Multiply src buffer with value and set to dst */
template <typename FloatType>
void multiply_set(FloatType* dst, const FloatType* src, const FloatType value, const int n) noexcept
{
    for (int i = 0; i < n; ++i)
        dst[i] = src[i] * value;
}

/** In-place sin */
template <typename FloatType>
void sin(FloatType* buf, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] = std::sin(buf[i]);
}

/** In-place cosine */
template <typename FloatType>
void cos(FloatType* buf, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] = std::cos(buf[i]);
}

/** Copy from source to destination */
template <typename FloatType>
void copy(FloatType* dst, FloatType* src, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        dst[i] = src[i];
}
    
/** Copy every ratioth sample from source to destination */
template <typename FloatType>
void copy_decimating(FloatType* dst, FloatType* src, const int ratio, const int n) noexcept
{
    for (int i=0, j=0; i < n; ++i, j+=ratio)
        dst[i] = src[j];
}

/** Add from source to destination */
template <typename FloatType>
void add(FloatType* dst, FloatType* src, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        dst[i] += src[i];
}

/** Add constant */
template <typename FloatType>
void add(FloatType* buf, FloatType value, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] += value;
}

/** Substract source from destination */
template <typename FloatType>
void substract(FloatType* dst, FloatType* src, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        dst[i] -= src[i];
}

/** Set to constant */
template <typename FloatType>
void set(FloatType* buf, FloatType value, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] = value;
}

/** Set to constant */
template <typename FloatType>
void clear(FloatType* buf, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] = 0;
}
    
    
/** Divide by sum of abs(buf)  */
template <typename FloatType>
void normalise_energy(FloatType* buf, const int n) noexcept
{
    float sum = 0;
    for (int i = 0; i < n; ++i)
        sum += abs(buf[i]);
    for (int i = 0; i < n; ++i)
        buf[i] /= sum;
}
    
template <typename FloatType>
void log(FloatType* buf, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] = std::log(buf[i]);
}

} // namespace math
} // namespace puro
