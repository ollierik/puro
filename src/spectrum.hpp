#pragma once

namespace puro {
    
template <typename BufferType>
void rfft(BufferType buffer, math::fft& fft) noexcept
{
    rfft(buffer, buffer, fft);
}

template <typename BufferType>
void rfft(BufferType dst, BufferType src, math::fft& fft) noexcept
{
    errorif(dst.length() != fft.length(), "dst length and fft size don't match");
    errorif(dst.length() != src.length(), "dst and source lengths don't match");

    for (auto ch=0; ch<dst.num_channels(); ++ch)
    {
        fft.rfft(dst[ch], src[ch]);
    }
}
    
template <typename BufferType, bool Normalise=true>
void irfft(BufferType buffer, math::fft& fft) noexcept
{
    irfft(buffer, buffer, fft);
}

template <typename BufferType, bool Normalise=true>
void irfft(BufferType dst, BufferType src, math::fft& fft) noexcept
{
    errorif(dst.length() != fft.length(), "dst length and fft size don't match");
    errorif(dst.length() != src.length(), "dst and source lengths don't match");
    
    for (auto ch=0; ch<dst.num_channels(); ++ch)
    {
        fft.irfft(dst[ch], src[ch]);
    }
    
    if (Normalise)
    {
        using T = typename BufferType::value_type;
        multiply(dst, 1 / static_cast<T>(fft.length()));
    }
}

template <typename DestBufferType, typename SourceBufferType>
void spectrum_magnitudes(DestBufferType dstReal, SourceBufferType srcComplex)
{
    errorif(dstReal.num_channels() != srcComplex.num_channels(), "channel configs not identicalt");
    errorif(dstReal.length() != (srcComplex.length() / 2 + 1), "buffer lengths are not compatible");
    
    using FloatType = typename DestBufferType::value_type;

    for (int ch=0; ch < dstReal.num_channels(); ++ch)
    {
        FloatType* src = srcComplex[ch];
        FloatType* dst = dstReal[ch];
        
        dst[0] = std::abs(src[0]);
        dst[dstReal.length() - 1] = std::abs(src[1]);

        for (int si=2, di=1; si < srcComplex.length(); si+=2, ++di)
        {
            dst[di] = std::hypot(src[si], src[si+1]);
        }
    }
}
    
template <typename BufferType>
void spectrum_phases(BufferType dstReal, BufferType srcComplex)
{
    errorif(dstReal.num_channels() != srcComplex.num_channels(), "channel configs not identicalt");
    errorif(dstReal.length() != (srcComplex.length() / 2 + 1), "buffer lengths are not compatible");

    using FloatType = typename BufferType::value_type;

    for (int ch=0; ch < dstReal.num_channels(); ++ch)
    {
        FloatType* src = srcComplex.channel(ch);
        FloatType* dst = dstReal.channel(ch);

        dst[0] = ( (src[0] >= 0) ? 0 : math::pi);
        dst[dstReal.length() - 1] = ( (src[1] >= 0) ? 0 : math::pi);

        for (int si=2, di=1; si < srcComplex.length(); si+=2, ++di)
        {
            const FloatType re = src[si];
            const FloatType im = src[si+1];
            dst[di] = std::atan2(im, re);
        }
    }
}

template <typename BufferType>
void spectrum_from_polar(BufferType dstComplex, BufferType magnitudesReal, BufferType phasesReal)
{
    errorif(magnitudesReal.length() != phasesReal.length(), "magnitudes and phases length differs");
    errorif(magnitudesReal.length() != dstComplex.length()/2+1, "dst and magnitudes lengths incompatible");
    
    using FloatType = typename BufferType::value_type;

    for (int ch=0; ch < dstComplex.num_channels(); ++ch)
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
    
template <typename BT1, typename BT2>
void spectrum_linphase_from_magnitudes(BT1 dstComplex, BT2 magnitudesReal)
{
    errorif(magnitudesReal.length() != dstComplex.length()/2+1, "dst and magnitudes lengths incompatible");
    
    using T = typename BT1::value_type;
    
    for (int ch=0; ch < dstComplex.num_channels(); ++ch)
    {
        T* dst = dstComplex[ch];
        T* src = magnitudesReal[ch];
        const int nq = magnitudesReal.length()-1;
        
        dst[0] = src[0];
        dst[1] = src[nq];
        
        for (int di=2, si=1; di < dstComplex.length(); di += 2, si += 1)
        {
            dst[di] = src[si];
            dst[di+1] = 0.0;
        }
    }
}
    
template <typename BufferType, typename MultBufferType>
void spectrum_substract(BufferType dst, const MultBufferType src) noexcept
{
    using FloatType = typename BufferType::value_type;
    using ComplexType = std::complex<FloatType>;

    for (int ch=0; ch<dst.num_channels(); ++ch)
    {
        ComplexType* dst_cplx = reinterpret_cast<ComplexType*> (dst[ch]);
        ComplexType* src_cplx = reinterpret_cast<ComplexType*> (src[ch]);

        for (auto i=0; i<dst.length()/2; ++i)
        {
            dst_cplx[i] -= src_cplx[i];
        }
    }
}
    
template <typename BT1, typename BT2>
void spectrum_multiply(BT1 dst, const BT2 src) noexcept
{
    using FloatType = typename BT1::value_type;
    using ComplexType = std::complex<FloatType>;

    if (dst.num_channels() == src.num_channels())
    {
        for (int ch=0; ch<dst.num_channels(); ++ch)
        {
            ComplexType* dst_cplx = reinterpret_cast<ComplexType*> (dst[ch]);
            ComplexType* src_cplx = reinterpret_cast<ComplexType*> (src[ch]);
    
            dst_cplx[0] = { src_cplx[0].real() * dst_cplx[0].real(), src_cplx[0].imag() * dst_cplx[0].imag() };

            for (auto i=1; i<dst.length()/2; ++i)
            {
                dst_cplx[i] *= src_cplx[i];
            }
        }
    }
    else if (src.num_channels() == 1)
    {
        for (int ch=0; ch<dst.num_channels(); ++ch)
        {
            ComplexType* dst_cplx = reinterpret_cast<ComplexType*> (dst[ch]);
            ComplexType* src_cplx = reinterpret_cast<ComplexType*> (src[0]);
            
            dst_cplx[0] = { src_cplx[0].real() * dst_cplx[0].real(), src_cplx[0].imag() * dst_cplx[0].imag() };
            
            for (auto i=1; i<dst.length()/2; ++i)
            {
                dst_cplx[i] *= src_cplx[i];
            }
        }
    }
    else
    {
        errorif(true, "incompatible channel configs");
    }
}

template <typename BT1, typename BT2, typename BT3>
void spectrum_multiply(BT1 dst, const BT2 src1, const BT3 src2) noexcept
{
    using FloatType = typename BT1::value_type;
    using ComplexType = std::complex<FloatType>;
    
    if (src1.num_channels() == src2.num_channels())
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            ComplexType* dst_cplx = reinterpret_cast<ComplexType*> (dst[ch]);
            ComplexType* src1_cplx = reinterpret_cast<ComplexType*> (src1[ch]);
            ComplexType* src2_cplx = reinterpret_cast<ComplexType*> (src2[ch]);

            dst_cplx[0] = { src1_cplx[0].real() * src2_cplx[0].real(), src1_cplx[0].imag() * src2_cplx[0].imag() };
    
            for (auto i=1; i<dst.length()/2; ++i)
            {
                dst_cplx[i] = src1_cplx[i] * src2_cplx[i];
            }
        }
    }
    else if (src2.num_channels() == 1)
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            ComplexType* dst_cplx = reinterpret_cast<ComplexType*> (dst[ch]);
            ComplexType* src1_cplx = reinterpret_cast<ComplexType*> (src1[ch]);
            ComplexType* src2_cplx = reinterpret_cast<ComplexType*> (src2[0]);
            
            dst_cplx[0] = { src1_cplx[0].real() * src2_cplx[0].real(), src1_cplx[0].imag() * src2_cplx[0].imag() };
            
            for (auto i=1; i<dst.length()/2; ++i)
            {
                dst_cplx[i] = src1_cplx[i] * src2_cplx[i];
            }
        }
    }
    else
    {
        errorif(true, "incompatible channel configs");
    }
}
    
template <typename BufferType>
void spectrum_exp(BufferType buf) noexcept
{
    using FloatType = typename BufferType::value_type;
    using ComplexType = std::complex<FloatType>;

    for (int ch=0; ch < buf.num_channels(); ++ch)
    {
        ComplexType* cplx = reinterpret_cast<ComplexType*> (buf[ch]);
        
        cplx[0] = { std::exp(cplx[0].real()), std::exp(cplx[0].imag()) };

        for (auto i=1; i < buf.length()/2; ++i)
        {
            cplx[i] = std::exp(cplx[i]);
        }
    }
}

/*
template <typename BufferType, typename DivBufferType>
void buffer_complex_divide(BufferType dst, const DivBufferType src) noexcept
{
    using ValueType = typename Buffer::value_type;
    
    for (int ch=0; ch<dst.num_channels(); ++ch)
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
    
    for (int ch=0; ch<dst.num_channels(); ++ch)
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
