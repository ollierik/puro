#pragma once

namespace puro {
    
    
template <typename BufferType>
void buffer_rfft_inplace(BufferType buffer, math::FFT& fft) noexcept
{
    errorif(buffer.length() != fft.size(), "buffer length and fft size don't match");
    for (auto ch=0; ch<buffer.getNumChannels(); ++ch)
    {
        fft.rfft(buffer.channel(ch), buffer.channel(ch));
    }
}

template <typename BufferType>
void buffer_irfft_inplace(BufferType buffer, math::FFT& fft) noexcept
{
    errorif(buffer.length() != fft.size(), "buffer length and fft size don't match");
    for (auto ch=0; ch<buffer.getNumChannels(); ++ch)
    {
        fft.irfft(buffer.channel(ch), buffer.channel(ch));
    }
}

template <typename BufferType>
void buffer_spectrum_magnitudes(BufferType dstReal, BufferType srcComplex)
{
    errorif(dstReal.getNumChannels() != srcComplex.getNumChannels(), "channel configs not identicalt");
    errorif(dstReal.length() != (srcComplex.length() / 2 + 1), "buffer lengths are not compatible");
    
    using FloatType = typename BufferType::value_type;

    for (int ch=0; ch < dstReal.getNumChannels(); ++ch)
    {
        FloatType* src = srcComplex.channel(ch);
        FloatType* dst = dstReal.channel(ch);
        
        dst[0] = std::abs(src[0]);
        dst[dstReal.length() - 1] = std::abs(src[1]);

        for (int si=2, di=1; si < srcComplex.length(); si+=2, ++di)
        {
            dst[di] = std::hypot(src[si], src[si+1]);
        }
    }
}
    
template <typename BufferType>
void buffer_spectrum_phases(BufferType dstReal, BufferType srcComplex)
{
    errorif(dstReal.getNumChannels() != srcComplex.getNumChannels(), "channel configs not identicalt");
    errorif(dstReal.length() != (srcComplex.length() / 2 + 1), "buffer lengths are not compatible");

    using FloatType = typename BufferType::value_type;

    for (int ch=0; ch < dstReal.getNumChannels(); ++ch)
    {
        FloatType* src = srcComplex.channel(ch);
        FloatType* dst = dstReal.channel(ch);

        dst[0] = ( (src[0] >= 0) ? 0 : math::pi<FloatType>());
        dst[dstReal.length() - 1] = ( (src[1] >= 0) ? 0 : math::pi<FloatType>());

        for (int si=2, di=1; si < srcComplex.length(); si+=2, ++di)
        {
            const FloatType re = src[si];
            const FloatType im = src[si+1];
            dst[di] = std::atan2(im, re);
        }
    }
}

template <typename BufferType>
void buffer_spectrum_from_polar(BufferType dstComplex, BufferType magnitudesReal, BufferType phasesReal)
{
    errorif(magnitudesReal.length() != phasesReal.length(), "magnitudes and phases length differs");
    errorif(magnitudesReal.length() != dstComplex.length()/2+1, "dst and magnitudes lengths incompatible");
    
    using FloatType = typename BufferType::value_type;

    for (int ch=0; ch < dstComplex.getNumChannels(); ++ch)
    {
        FloatType* dst = dstComplex.channel(ch);
        FloatType* msrc = magnitudesReal.channel(ch);
        FloatType* psrc = phasesReal.channel(ch);
        const int nq = magnitudesReal.length()-1;
        
        dst[0] = (psrc[0] == 0) ? msrc[0] : -msrc[0];
        dst[1] = (psrc[nq] == 0) ? msrc[nq] : -msrc[nq];

        for (int di=2, si=1; di < dstComplex.length(); di += 2, ++si)
        {
            dst[di] = std::cos(psrc[si]) * msrc[si];
            dst[di + 1] = std::sin(psrc[si]) * msrc[si];
        }
    }
}
    
/*
template <typename BufferType, typename MultBufferType>
void buffer_complex_multiply(BufferType dst, const MultBufferType src) noexcept
{
    using ValueType = typename Buffer::value_type;

    for (int ch=0; ch<dst.getNumChannels(); ++ch)
    {
        ValueType* dptr = dst.channel(ch);
        ValueType* sptr = src.channel(ch);
        
        dptr[0] *= sptr[0];
        dptr[1] *= sptr[1];

        for (int i=2; i<dst.length(); i+=2)
        {
            std::complex<float> dc { dptr[i], dptr[i+1] };
            std::complex<float> sc { sptr[i], sptr[i+1] };
            dc *= sc;
            dptr[i] = dc.real;
            dptr[i+1] = dc.imag;
        }
    }
}

template <typename BufferType, typename DivBufferType>
void buffer_complex_divide(BufferType dst, const DivBufferType src) noexcept
{
    using ValueType = typename Buffer::value_type;
    
    for (int ch=0; ch<dst.getNumChannels(); ++ch)
    {
        ValueType* dptr = dst.channel(ch);
        ValueType* sptr = src.channel(ch);
        
        dptr[0] *= sptr[0];
        dptr[1] *= sptr[1];
        
        for (int i=2; i<dst.length(); i+=2)
        {
            std::complex<float> dc { dptr[i], dptr[i+1] };
            std::complex<float> sc { sptr[i], sptr[i+1] };
            dc /= sc;
            dptr[i] = dc.real;
            dptr[i+1] = dc.imag;
        }
    }
}

template <typename BufferType>
void buffer_complex_normalise(BufferType buffer) noexcept
{
    using ValueType = typename Buffer::value_type;
    
    for (int ch=0; ch<dst.getNumChannels(); ++ch)
    {
        ValueType* ptr = buffer.channel(ch);

        ptr[0] /= std::abs(ptr[0]);
        ptr[1] /= std::abs(ptr[1]);
        
        for (int i=2; i<dst.length(); i+=2)
        {
            std::complex<float> c { ptr[i], ptr[i+1] };
            c /= std::abs(c);
            ptr[i] = c.real;
            ptr[i+1] = c.imag;
        }
    }
}
 */

 
} // namespace puro
