#pragma once

namespace puro {

template <typename BT1, typename BT2>
inline void spectrum_power(BT1 dstReal, BT2 srcComplex)
{
    errorif(dstReal.num_channels() != srcComplex.num_channels(), "channel configs not identicalt");
    errorif(dstReal.length() != (srcComplex.length() / 2 + 1), "buffer lengths are not compatible");

    typedef typename BT1::value_type T;

    for (int ch=0; ch < dstReal.num_channels(); ++ch)
    {
        T* src = srcComplex.channel(ch);
        T* dst = dstReal.channel(ch);

        dst[0] = fabs(src[0]);
        dst[dstReal.length() - 1] = fabs(src[1]);

        for (int si=2, di=1; si < srcComplex.length(); si+=2, ++di)
        {
            dst[di] = src[si]*src[si] + src[si+1]*src[si+1];
        }
    }
}

template <typename BT1, typename BT2>
void spectrum_magnitudes(BT1 dstReal, BT2 srcComplex)
{
    errorif(dstReal.num_channels() != srcComplex.num_channels(), "channel configs not identicalt");
    errorif(dstReal.length() != (srcComplex.length() / 2 + 1), "buffer lengths are not compatible");
    
    typedef typename BT1::value_type T;

    for (int ch=0; ch < dstReal.num_channels(); ++ch)
    {
        T* src = srcComplex.channel(ch);
        T* dst = dstReal.channel(ch);
        
        dst[0] = fabs(src[0]);
        dst[dstReal.length() - 1] = fabs(src[1]);

        for (int si=2, di=1; si < srcComplex.length(); si+=2, ++di)
        {
            dst[di] = sqrtf_ti(src[si]*src[si] + src[si+1] * src[si+1]);
        }
    }
}
    
template <typename BT1, typename BT2>
inline void spectrum_phases(BT1 dstReal, BT2 srcComplex)
{
    errorif(dstReal.num_channels() != srcComplex.num_channels(), "channel configs not identicalt");
    errorif(dstReal.length() != (srcComplex.length() / 2 + 1), "buffer lengths are not compatible");

    typedef typename BT1::value_type T;

    for (int ch=0; ch < dstReal.num_channels(); ++ch)
    {
        T* src = srcComplex.channel(ch);
        T* dst = dstReal.channel(ch);

        dst[0] = ( (src[0] >= 0) ? 0 : math::pi);
        dst[dstReal.length() - 1] = ( (src[1] >= 0) ? 0 : math::pi);

        for (int si=2, di=1; si < srcComplex.length(); si+=2, ++di)
        {
            const T re = src[si];
            const T im = src[si+1];
            dst[di] = std::atan2(im, re);
        }
    }
}

template <typename BT1, typename BT2, typename BT3>
inline void spectrum_from_polar(BT1 dstComplex, BT2 magnitudesReal, BT3 phasesReal)
{
    errorif(magnitudesReal.length() != phasesReal.length(), "magnitudes and phases length differs");
    errorif(magnitudesReal.length() != dstComplex.length()/2+1, "dst and magnitudes lengths incompatible");
    
    typedef typename BT1::value_type T;

    for (int ch=0; ch < dstComplex.num_channels(); ++ch)
    {
        T* dst = dstComplex.channel(ch);
        T* msrc = magnitudesReal.channel(ch);
        T* psrc = phasesReal.channel(ch);
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
inline void spectrum_linphase_from_magnitudes(BT1 dstComplex, BT2 magnitudesReal)
{
    errorif(magnitudesReal.length() != dstComplex.length()/2+1, "dst and magnitudes lengths incompatible");
    
    typedef typename BT1::value_type T;
    
    for (int ch=0; ch < dstComplex.num_channels(); ++ch)
    {
        T* dst = dstComplex.channel(ch);
        T* src = magnitudesReal.channel(ch);
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
    
template <typename BT1, typename BT2>
inline void spectrum_substract(BT1 dst, const BT2 src)
{
    typedef typename BT1::value_type T;

    for (int ch=0; ch<dst.num_channels(); ++ch)
    {
        math::substract(dst.channel(ch), src.channel(ch), dst.length());
    }
}
    
template <typename BT1, typename BT2>
inline void spectrum_multiply(BT1 dst, const BT2 src)
{
    errorif (dst.length() != src.length(), "lengths don't match");
    
    typedef typename BT1::value_type T;

    if (dst.num_channels() == src.num_channels())
    {
        for (int ch=0; ch<dst.num_channels(); ++ch)
        {
            T* dptr = dst.channel(ch);
            T* sptr = src.channel(ch);
            
            dptr[0] *= sptr[0];
            dptr[1] *= sptr[1];

            math::complex_multiply(&dptr[2], &sptr[2], dst.length()-2);
        }
    }
    else if (src.num_channels() == 1)
    {
        for (int ch=0; ch<dst.num_channels(); ++ch)
        {
            T* dptr = dst.channel(ch);
            T* sptr = src.channel(0);

            dptr[0] *= sptr[0];
            dptr[1] *= sptr[1];
            
            math::complex_multiply(&dptr[2], &sptr[2], dst.length()-2);
        }
    }
    else
    {
        errorif(true, "incompatible channel configs");
    }
}

template <typename BT1, typename BT2, typename BT3>
inline void spectrum_multiply(BT1 dst, const BT2 src1, const BT3 src2)
{
    typedef typename BT1::value_type T;

    errorif (dst.length() != src1.length(), "lengths don't match");
    errorif (src1.length() != src2.length(), "lengths don't match");
    
    if (src1.num_channels() == src2.num_channels())
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            T* dptr = dst.channel(ch);
            T* sptr1 = src1.channel(ch);
            T* sptr2 = src2.channel(ch);
            
            dptr[0] = sptr1[0] * sptr2[0];
            dptr[1] = sptr1[1] * sptr2[1];

            math::complex_multiply(&dptr[2], &sptr1[2], &sptr2[2], dst.length()-2);
        }
    }
    else if (src2.num_channels() == 1)
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            T* dptr = dst.channel(ch);
            T* sptr1 = src1.channel(ch);
            T* sptr2 = src2.channel(0);

            dptr[0] = sptr1[0] * sptr2[0];
            dptr[1] = sptr1[1] * sptr2[1];
            
            math::complex_multiply(&dptr[2], &sptr1[2], &sptr2[2], dst.length()-2);
        }
    }
    else
    {
        errorif(true, "incompatible channel configs");
    }
}

} // namespace puro
