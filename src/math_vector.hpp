#pragma once

/** Maths routines, mostly for buffers. Used to allow flexibility later on by implementing vector math libs such as IPP */
namespace puro {
namespace math {

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
inline void multiply(FloatType* dst, const FloatType value, const int n) noexcept
{
    for (int i = 0; i < n; ++i)
        dst[i] *= value;
};

template <typename FloatType>
inline void multiply(FloatType* PURO_RESTRICT dst, const FloatType* PURO_RESTRICT src, const int n) noexcept
{
    for (int i = 0; i < n; ++i)
        dst[i] *= src[i];
};

template <typename FloatType>
inline void multiply_add(FloatType* PURO_RESTRICT dst, const FloatType* PURO_RESTRICT src1, const FloatType* PURO_RESTRICT src2, const int n) noexcept
{
    for (int i = 0; i < n; ++i)
        dst[i] += src1[i] * src2[i];
};

template <typename FloatType>
inline void multiply_add(FloatType* PURO_RESTRICT dst, const FloatType* PURO_RESTRICT src, const FloatType value, const int n) noexcept
{
    for (int i = 0; i < n; ++i)
        dst[i] += src[i] * value;
};

/** Multiply src buffer with value and set to dst */
template <typename FloatType>
inline void multiply(FloatType* PURO_RESTRICT dst, const FloatType* PURO_RESTRICT src, const FloatType value, const int n) noexcept
{
    for (int i = 0; i < n; ++i)
        dst[i] = src[i] * value;
}

/** In-place sin */
template <typename FloatType>
inline void sin(FloatType* buf, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] = std::sin(buf[i]);
}

/** In-place cosine */
template <typename FloatType>
inline void cos(FloatType* buf, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] = std::cos(buf[i]);
}
    
template <typename FloatType>
inline void osc(FloatType* buf, FloatType norm_freq, const int n)
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
inline void reciprocal(FloatType* buf, const int n)
{
    for (int i=0; i<n; ++i)
        buf[i] = ((FloatType)1) / buf[i];
}
    
template <typename FloatType>
inline void max(FloatType* buf, FloatType value, const int n)
{
    for (int i=0; i<n; ++i)
        buf[i] = buf[i] > value ? buf[i] : value;
}

/** Copy from source to destination */
template <typename FloatType>
inline void copy(FloatType* PURO_RESTRICT dst, FloatType* PURO_RESTRICT src, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        dst[i] = src[i];
}
    
/** Copy every ratioth sample from source to destination */
template <typename FloatType>
inline void copy_decimating(FloatType* PURO_RESTRICT dst, FloatType* PURO_RESTRICT src, const int stride, const int n) noexcept
{
    for (int i = 0, j = 0; i < n; ++i, j += stride)
        dst[i] = src[j];
}

/** Add from source to destination */
template <typename FloatType>
inline void add(FloatType* PURO_RESTRICT dst, FloatType* PURO_RESTRICT src, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        dst[i] += src[i];
}

/** Add constant */
template <typename FloatType>
inline void add(FloatType* buf, FloatType value, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] += value;
}

/** Substract source from destination */
template <typename FloatType>
inline void substract(FloatType* dst, FloatType* src, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        dst[i] -= src[i];
}

/** Set to constant */
template <typename FloatType>
inline void set(FloatType* buf, FloatType value, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] = value;
}

/** Set to 0 */
template <typename FloatType>
inline void clear(FloatType* buf, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] = 0;
}
    
template <typename FloatType>
inline void pow(FloatType* buf, FloatType power, const int n)
{
    for (int i=0; i<n; ++i)
        buf[i] = std::pow(buf[i], power);
}
    
template <typename FloatType>
inline void log(FloatType* buf, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] = std::log(buf[i]);
}


template <typename FloatType>
inline FloatType sum(FloatType* buf, const int n) noexcept
{
    FloatType sigma = 0;
    
    for (int i=0; i<n; ++i)
        sigma += buf[i];
    
    return sigma;
}
    
template <typename FloatType>
inline FloatType abssum(FloatType* buf, const int n) noexcept
{
    FloatType sigma = 0;

    for (int i=0; i<n; ++i)
        sigma += std::abs(buf[i]);

    return sigma;
}
    
/** Add constant */
template <typename FloatType>
inline void clip_low(FloatType* buf, FloatType value, const int n) noexcept
{
    for (int i=0; i<n; ++i)
        buf[i] = max(buf[i], value);
}
    
/** Divide by sum of abs(buf)  */
template <typename FloatType>
inline void normalise_energy(FloatType* buf, const int n) noexcept
{
    float sum = 0;
    for (int i = 0; i < n; ++i)
        sum += abs(buf[i]);
    if (sum != 0)
    {
        for (int i = 0; i < n; ++i)
            buf[i] /= sum;
    }
}
    
// COMPLEX
    
/** Multiply src buffer with value and set to dst */
template <typename T>
inline void complex_multiply(T* PURO_RESTRICT dst, const T* PURO_RESTRICT src, const int n)
{
    for (int i=0; i < n; i += 2)
    {
        const T a = dst[i];
        const T b = dst[i+1];
        const T c = src[i];
        const T d = src[i+1];

        dst[i]   = a * c - b * d;
        dst[i+1] = a * d + b * c;
    }
}

template <typename T>
inline void complex_multiply(T* PURO_RESTRICT dst, const T* PURO_RESTRICT src1, const T* PURO_RESTRICT src2, const int n)
{
    for (int i=0; i < n; i += 2)
    {
        const T a = src1[i];
        const T b = src1[i+1];
        const T c = src2[i];
        const T d = src2[i+1];

        dst[i]   = a * c - b * d;
        dst[i+1] = a * d + b * c;
    }
}
    
/** FFT wrapper for pffft. Results are not normalised!
 irfft(rfft(signal)) = fftSize * signal */
struct fft
{
    fft(int size) : setup(pffft_new_setup(size, PFFFT_REAL)), fft_size(size)
    {}

    ~fft()
    {
        pffft_destroy_setup(setup);
    }

    template <typename BT>
    void rfft(BT buffer)
    {
        for (int ch=0; ch<buffer.num_channels(); ++ch)
        {
            pffft_transform_ordered(setup, buffer.channel(ch), buffer.channel(ch), 0, PFFFT_FORWARD);
        }
    }

    template <typename BT1, typename BT2>
    void rfft(BT1 dst, BT2 src)
    {
        errorif(dst.num_channels() != src.num_channels(), "number of channels not same");
        for (int ch=0; ch < dst.num_channels(); ++ch)
        {
            pffft_transform_ordered(setup, src.channel(ch), dst.channel(ch), 0, PFFFT_FORWARD);
        }
    }

    template <typename BT, bool Normalise = true>
    void irfft(BT buffer)
    {
        for (int ch=0; ch<buffer.num_channels(); ++ch)
        {
            pffft_transform_ordered(setup, buffer.channel(ch), buffer.channel(ch), 0, PFFFT_BACKWARD);
            if (Normalise)
                math::multiply(buffer.channel(ch), 1.0f/(float)fft_size, buffer.length());
        }

    }

    template <typename BT1, typename BT2, bool Normalise = true>
    void irfft(BT1 dst, BT2 src)
    {
        errorif(dst.num_channels() != src.num_channels(), "number of channels not same");
        for (int ch=0; ch < dst.num_channels(); ++ch)
        {
            pffft_transform_ordered(setup, src.channel(ch), dst.channel(ch), 0, PFFFT_BACKWARD);
            if (Normalise)
                math::multiply(dst.channel(ch), 1.0f/(float)fft_size, dst.length());
        }
    }

    int length() const
    {
        return fft_size;
    }

    PFFFT_Setup* setup;
    int fft_size;
};

} // namespace math
} // namespace puro
