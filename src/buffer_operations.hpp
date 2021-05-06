#pragma once

namespace puro {

template <typename BufferType>
BufferType buffer_trim_begin(BufferType buffer, int offset)
{
    errorif(offset < 0 || offset > buffer.numSamples, "offset out of bounds");

    buffer.numSamples -= offset;

    for (int ch = 0; ch < buffer.num_channels(); ++ch)
        buffer.channelPtrs[ch] = &buffer.channelPtrs[ch][offset];

        return buffer;
}


template <typename BufferType>
BufferType buffer_trim_length(BufferType buffer, int newLength)
{
    errorif(newLength > buffer.numSamples, "new length out of bounds");

    buffer.numSamples = math::max(newLength, 0);
    return buffer;
}

/// Get a sub segment of a buffer with given offset and length
template <typename BT>
BT segment(BT buffer, int offset, int length)
{
    errorif(offset > buffer.numSamples, "segment offset greater than number of samples available");
    errorif(length < 0 || length > (offset + buffer.length()), "segment length out of bounds");

    for (int ch = 0; ch < buffer.num_channels(); ++ch)
        buffer.channelPtrs[ch] = &buffer.channelPtrs[ch][offset];

    buffer.numSamples = length;
    return buffer;
}

/// Get a segment of a buffer from start to end index
template <typename BT>
BT slice(BT buffer, int start, int end)
{
    errorif(start < 0, "slice start below zero");
    errorif(start > buffer.length(), "slice start greater than number of samples available");
    errorif(end < start, "slice end below start");
    errorif(end > buffer.length(), "slice end greater than number of samples available");

    for (int ch = 0; ch < buffer.num_channels(); ++ch)
        buffer.ptrs[ch] = &buffer.ptrs[ch][start];

        buffer.num_samples = end - start;
        return buffer;
}

/*
/// Split the given buffer into from index. The second buffer starts with index at zeroeth index.
template <typename BT>
std::tuple<BT, BT> split(BT buffer, int index)
{
    errorif(index <= 0, "split is 0 or below");
    errorif(index >= buffer.numSamples, "split greater than number of samples available");

    BT pre = buffer_trim_length(buffer, index);
    BT post = buffer_trim_begin(buffer, index);

    return std::make_tuple(std::move(pre), std::move(post));
}
     */

template <typename BT1, typename BT2, typename BT3>
inline
typename enable_if_buffer<BT3, void>::type
multiply_add(const BT1 dst, const BT2 src1, const BT3 src2)
{
    errorif(!(dst.length() == src1.length()), "dst and src1 buffer lengths don't match");
    errorif(!(dst.length() == src2.length()), "dst and src2 buffer lengths don't match");

    // identical channel configs
    if (src1.num_channels() == src2.num_channels())
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::multiply_add(dst.channel(ch), src1.channel(ch), src2.channel(ch), dst.length());
        }
    }
    // src2 is a mono buffer
    else if (src1.num_channels() > 1 && src2.num_channels() == 1)
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::multiply_add(dst.channel(ch), src1.channel(ch), src2.channel(0), dst.length());
        }
    }
    else
    {
        errorif(true, "channel config not implemented");
    }
}

template <typename BT1, typename BT2, typename ValueType>
inline
typename enable_if_scalar<ValueType, void>::type
multiply_add(const BT1 dst, const BT2 src, const ValueType multiplier)
{
    errorif(dst.num_channels() != src.num_channels(), "dst and src channel number doesn't match");
    errorif(dst.length() != src.length(), "dst and src1 buffer lengths don't match");

    // identical channel configs
    for (int ch = 0; ch < dst.num_channels(); ++ch)
    {
        math::multiply_add(dst.channel(ch), src.channel(ch), multiplier, dst.length());
    }
}

template <typename BT>
inline void multiply(BT dst, const typename BT::value_type value)
{
    for (int ch = 0; ch < dst.num_channels(); ++ch)
    {
        math::multiply(dst.channel(ch), value, dst.length());
    }
}

template <typename BT1, typename BT2>
inline
typename enable_if_buffer<BT2, void>::type
multiply(BT1 dst, const BT2 src)
{
    errorif(dst.length() != src.length(), "dst and src buffer lengths don't match");

    // identical channel config
    if (dst.num_channels() == src.num_channels())
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::multiply(dst.channel(ch), src.channel(ch), dst.length());
        }
    }
    // mono src, multichannel dst
    else if (dst.num_channels() != src.num_channels() && src.num_channels() == 1)
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::multiply(dst.channel(ch), src.channel(0), dst.length());
        }
    }
}

template <typename BT1, typename BT2>
inline void multiply(BT1 dst, BT2 src, typename BT1::value_type value)
{
    errorif(dst.length() != src.length(), "dst and src buffer lengths don't match");

    // identical channel config
    for (int ch = 0; ch < dst.num_channels(); ++ch)
    {
        math::multiply(dst.channel(ch), src.channel(ch), value, dst.length());
    }
}

template <typename BT1, typename BT2>
inline
typename enable_if_buffer<BT2, void>::type
add(BT1 dst, const BT2 src)
{
    errorif(dst.length() != src.length(), "dst and src buffer lengths don't match");

    // identical channel config
    if (dst.num_channels() == src.num_channels())
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::add(dst.channel(ch), src.channel(ch), dst.length());
        }
    }
    // mono src, multichannel dst
    else if (dst.num_channels() != src.num_channels() && src.num_channels() == 1)
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::add(dst.channel(ch), src.channel(0), dst.length());
        }
    }
}

template <typename BT>
inline void add(BT dst, const typename BT::value_type value)
{
    for (int ch = 0; ch < dst.num_channels(); ++ch)
    {
        math::add(dst.channel(ch), value, dst.length());
    }
}

template <typename BT1, typename BT2>
inline void substract(BT1 dst, const BT2 src)
{
    errorif(dst.length() != src.length(), "dst and src buffer lengths don't match");

    // identical channel config
    if (dst.num_channels() == src.num_channels())
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::substract(dst.channel(ch), src.channel(ch), dst.length());
        }
    }
    // mono src, multichannel dst
    else if (dst.num_channels() != src.num_channels() && src.num_channels() == 1)
    {
        for (int ch = 0; ch < dst.num_channels(); ++ch)
        {
            math::substract(dst.channel(ch), src.channel(0), dst.length());
        }
    }
}

template <typename BT1, typename BT2>
inline void copy(BT1 dst, const BT2 src)
{
    errorif(dst.num_channels() != src.num_channels(), "dst and src channel number doesn't match");
    errorif(dst.length() != src.length(), "dst and src lengths don't match");

    for (int ch=0; ch<dst.num_channels(); ++ch)
    {
        math::copy(dst.channel(ch), src.channel(ch), dst.length());
    }
}

template <typename BT1, typename BT2>
inline void copy_decimating(BT1 dst, BT2 src, int stride)
{
    errorif(dst.length() != (src.length() / stride), "dst.length (" << dst.length() << ") should be src.length() / stride" << src.length() << "/" << stride << ") = " << src.length() / stride);
    errorif(src.length() % stride != 0, "src.length should be divisible by ratio");

    for (int ch=0; ch<dst.num_channels(); ++ch)
    {
        math::copy_decimating(dst.channel(ch), src.channel(ch), stride, src.length());
    }
}
    
template <typename BT1, typename BT2>
inline void copy_downmixing (BT1 dst, const BT2 src)
{
    errorif(dst.num_channels() != 1, "dst it not a mono buffer");
    errorif(dst.length() != src.length(), "dst and src lengths don't match");

    math::copy(dst.channel(0), src.channel(0), dst.length());
    
    for (int ch=1; ch<dst.num_channels(); ++ch)
    {
        math::copy(dst.channel(0), src.channel(ch), dst.length());
    }
}

template <typename BT>
inline void clear(BT buffer)
{
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        math::clear(buffer.channel(ch), buffer.length());
    }
}

template <typename BT>
inline void normalise(BT buffer)
{
    typedef typename BT::value_type T;

    T max = 0;
    for (int ch = 0; ch < buffer.num_channels(); ++ch)
    {
        T* ptr = buffer.channel(ch);

        for (int i = 0; i < buffer.length(); ++i)
        {
            max = math::max(abs(ptr[i]), max);
        }
    }

    if (max > 0)
    {
        for (int ch = 0; ch < buffer.num_channels(); ++ch)
        {
            T* ptr = buffer.channel(ch);
            const float mult = (T)1/max;
            for (int ch=0; ch<buffer.num_channels(); ++ch)
            {
                math::multiply(ptr, mult, buffer.length());
            }
        }
    }
}

template <typename BT>
inline void max(BT buffer, const typename BT::value_type value)
{
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        math::max(buffer.channel(ch), value, buffer.length());
    }
}

template <typename BT>
inline void pow(BT buffer, const typename BT::value_type power)
{
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        math::pow(buffer.channel(ch), power, buffer.length());
    }
}

template <typename BT>
inline void reciprocal(BT buffer)
{
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        math::reciprocal(buffer.channel(ch), buffer.length());
    }
}

template <typename BT>
inline void log(BT buffer)
{
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        math::log(buffer.channel(ch), buffer.length());
    }
}

template <typename BT>
inline void negate(BT buffer)
{
    typedef typename BT::value_type T;
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        math::multiply(buffer.channel(ch), static_cast<T>(-1), buffer.length());
    }
}

template <typename BT>
typename BT::value_type sum(BT buffer) 
{
    typedef typename BT::value_type T;
    
    T sigma = 0;
    
    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        sigma = math::sum(buffer.channel(ch), buffer.length());
    }
                          
    return sigma;
}
    
template <typename BT>
typename BT::value_type abssum(BT buffer)
{
    typedef typename BT::value_type T;

    T sigma = 0;

    for (int ch=0; ch<buffer.num_channels(); ++ch)
    {
        sigma = math::abssum(buffer.channel(ch), buffer.length());
    }

    return sigma;
}
    
template <typename BT>
inline void clip_low(BT dst, const typename BT::value_type low)
{
    for (int ch = 0; ch < dst.num_channels(); ++ch)
    {
        math::clip_low(dst.channel(ch), low, dst.length());
    }
}
    

} // namespace puro
