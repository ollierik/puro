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
void multiply(FloatType* dst, const FloatType* src, const FloatType value, const int n) noexcept
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
    
template <typename FloatType>
void osc(FloatType* buf, FloatType norm_freq, const int n)
{
    const FloatType inc = norm_freq * 2 * pi;
    FloatType phase = 0;
    
    for (int i=0; i < n; i++)
    {
        buf[i] = std::cos(phase);
        phase += inc;
    }
}
    
template <typename FloatType>
void reciprocal(FloatType* buf, const int n)
{
    for (int i=0; i<n; ++i)
        buf[i] = ((FloatType)1) / buf[i];
}
    
template <typename FloatType>
void max(FloatType* buf, FloatType value, const int n)
{
    for (int i=0; i<n; ++i)
        buf[i] = buf[i] > value ? buf[i] : value;
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
void copy_decimating(FloatType* dst, FloatType* src, const int stride, const int n) noexcept
{
    for (int i = 0, j = 0; i < n; ++i, j += stride)
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

/** Set to 0 */
template <typename FloatType>
void clear(FloatType* buf, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] = 0;
}
    
template <typename FloatType>
void pow(FloatType* buf, FloatType power, const int n)
{
    for (int i=0; i<n; ++i)
        buf[i] = std::pow(buf[i], power);
}
    
template <typename FloatType>
void log(FloatType* buf, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] = std::log(buf[i]);
}


template <typename FloatType>
FloatType sum(FloatType* buf, const int n) noexcept
{
    FloatType sigma = 0;
    
    for (int i=0; i<n; ++i)
        sigma += buf[i];
    
    return sigma;
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

} // namespace math
} // namespace puro
